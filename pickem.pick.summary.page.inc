<?php
// $Id: $

/**
 * @file
 * Page callbacks and theme functions for the pick summary page.
 */

/**
 * Menu callback;
 */
function pick_summary_page($league, $week) {
  global $user;
  drupal_set_title(t('Pick Summary - ') . $week->wk_name);

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l($league->name, 'pickem/'. $league->lid);
  drupal_set_breadcrumb($breadcrumb);


  $teams = get_teams(array('gameset_id' => $league->gsid));
  $weeks = get_weeks(array('week_id' => $week->wid));
  $games = get_games(array('week_id' => $week->wid));
  $theme_data = array('games'=>array(), 'users'=>array());
  foreach ($games as $gid => $g_data) {

    // Save who is the home team and who is the visitor.
    $theme_data['games'][$gid]['h_id'] = $g_data->h_id;
    $theme_data['games'][$gid]['v_id'] = $g_data->v_id;

    // Load-up the home team info.
    $theme_data['games'][$gid]['teams'][$g_data->h_id] = array(
      'short_name' => $teams[$g_data->h_id]->short_name,
      'long_name' => $teams[$g_data->h_id]->long_name
    );

    // Load-up the visiting team info.
    $theme_data['games'][$gid]['teams'][$g_data->v_id] = array(
      'short_name' => $teams[$g_data->v_id]->short_name,
      'long_name' => $teams[$g_data->v_id]->long_name
    );

    // Load-up the game titles.
    $theme_data['games'][$gid]['short_game_title'] =
      $theme_data['games'][$gid]['teams'][$g_data->v_id]['short_name'] .
      ' @ ' .
      $theme_data['games'][$gid]['teams'][$g_data->h_id]['short_name'];

    $theme_data['games'][$gid]['long_game_title'] =
      $theme_data['games'][$gid]['teams'][$g_data->v_id]['long_name'] .
      ' @ ' .
      $theme_data['games'][$gid]['teams'][$g_data->h_id]['long_name'];



    // Figure out who won.
    if ( $g_data->h_score == NULL ) {
      $theme_data['games'][$gid]['winner'] = 0;  // game not played
    }
    elseif ($g_data->h_score == $g_data->v_score) {
      $theme_data['games'][$gid]['winner'] = -1; // game ended in tie
    }
    elseif ($g_data->h_score > $g_data->v_score) {
      $theme_data['games'][$gid]['winner'] = $g_data->h_id;  // home team won.
    }
    elseif ($g_data->v_score > $g_data->h_score) {
      $theme_data['games'][$gid]['winner'] = $g_data->v_id;  // visiting team won.
    }

    // Save the score (May be NULL).
    $theme_data['games'][$gid]['teams'][$g_data->h_id]['score'] = $g_data->h_score;
    $theme_data['games'][$gid]['teams'][$g_data->v_id]['score'] = $g_data->v_score;


  }

  $totals = get_user_totals($league, $week->wid); //sorted by weekly scores
  $picks = get_picks( array('league_id' => $league->lid, 'week_id' => $week->wid) );
  foreach ($totals as $uid => $tot) {
    $user_form = array(
      'uid' => $uid,
      'user_link' => theme('username', $tot['user_obj']),
      'weekly_rank' => $tot['ranking'],
      'weekly_points' => $tot['total'],
      'picks' => array()
    );

    // Loop collect picks and pick points.
    foreach ($theme_data['games'] as $gid => $game_info) {

      // Week has not yet started, don't show picks or scores unless auth user is the pick user.
      if ($week->week_past == 0) {
        if ( isset($picks[$gid][$uid]->winnerpick_id) && $picks[$gid][$uid]->winnerpick_id != 0) {
          if ( $uid == $user->uid ) {
            // User is allowed to see their own picks, but not others.
            $user_form['picks'][$gid]['picked'] = $picks[$gid][$uid]->winnerpick_id;
          }
          else {
            // Use 0 to mean "you are not allowed to see this."
            $user_form['picks'][$gid]['picked'] = 0;
          }
        }
        else {
          // Use a -1 to mean they haven't picked yet.
          // It also might mean they are using another pick system which may
          // use another field.
          $user_form['picks'][$gid]['picked'] = -1;
        }
      }

      // Week start has passed, go ahead and show picks and totals.
      else {
        // User is allowed to see all picks now.
        // If they didn't pick at all, use a -1.
        if ($picks[$gid][$uid]->winnerpick_id == 0) {
          $user_form['picks'][$gid]['picked'] = -1;
        }
        else {
          $user_form['picks'][$gid]['picked'] = $picks[$gid][$uid]->winnerpick_id;
        }

        if ( !isset($tot['weeks'][$week->wid]['games'][$gid]) ) {
          $user_form['picks'][$gid]['points'] = 0;
        }
        else {
          $user_form['picks'][$gid]['points'] = $tot['weeks'][$week->wid]['games'][$gid];
        }
      }
    }

    // Since we are using push they should stay in ranking order.
    array_push($theme_data['users'], $user_form);
  }

  return theme('pick_summary', $theme_data);
}


function theme_pick_summary($theme_data) {

  // Pager.
  global $pager_page_array, $pager_total;
  $page = isset($_GET['page']) ? $_GET['page'] : 0;
  $pager_page_array = explode(',', $page);
  // how many pages.
  $items_per_page = variable_get('pickem_summary_items_per_page', 1000);
  $pager_total[0] = ceil(count($theme_data['games']) / $items_per_page);

  $header = array();
  $header[] = t('Weekly Rank');
  $header[] = t('Points');
  $i = 0;
  foreach ($theme_data['games'] as $gid => $game_info) {
    if ( $i >= $page * $items_per_page && $i < ($page+1) * $items_per_page ) {
      $short_game_title = str_replace(" @ ", "<br/>@<br/>", $theme_data['games'][$gid]['short_game_title']);
      $header[] = array('data' => $short_game_title, 'title' => $theme_data['games'][$gid]['long_game_title']);
    }
    $i++;
  }

  $rows = array();
  foreach (element_children($theme_data['users']) as $key) {
    $cols = array();
    $cols[] = array('data' => $theme_data['users'][$key]['weekly_rank'] . '. '. $theme_data['users'][$key]['user_link'], 'class'=>'users');
    $cols[] = $theme_data['users'][$key]['weekly_points'];

    $i = 0;
    foreach (element_children($theme_data['games']) as $gid) {
      if ( $i >= $page * $items_per_page && $i < ($page+1) * $items_per_page ) {
        $class = '';
        if ( $theme_data['games'][$gid]['winner'] == 0 ) {
          $class = 'not-played';
        } elseif ( $theme_data['users'][$key]['picks'][$gid]['points'] > 0 ) {
          $class = 'win';
        } elseif ( $theme_data['users'][$key]['picks'][$gid]['points'] == 0 ) {
          $class = 'loss';
        }

        $col = array('data' => '');
        if ( $theme_data['users'][$key]['picks'][$gid]['picked'] == 0 ) {
          $col['data'] .= 'X';
        }
        elseif ( $theme_data['users'][$key]['picks'][$gid]['picked'] == -1 ) {
          $col['data'] .= '-';
        }
        else {
          $col['data'] .= $theme_data['games'][$gid]['teams'][$theme_data['users'][$key]['picks'][$gid]['picked']]['short_name'];
        }
        $col['class'] = $class;

        if ( $class != 'not-played' && $theme_data['users'][$key]['picks'][$gid]['points'] >= 0 ) {
          $col['data'] .= ": " . $theme_data['users'][$key]['picks'][$gid]['points'];
        }

        $cols[] = $col;
      }

      $i++;
    }


    array_push($rows, $cols);
  }
  $output = '';
  $output .= theme('pager', array(), 20);
  $output .= theme('table', $header, $rows, array('class'=>'pickem-pick-summary'));

  return $output;
}
