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

  return drupal_get_form('pick_summary_form', $league, $week);
}


function pick_summary_form($form_state=NULL, $league, $week) {
  global $user;

  $teams = get_teams(array('gameset_id' => $league->gsid));
  $weeks = get_weeks(array('week_id' => $week->wid));
  $games = get_games(array('week_id' => $week->wid));
  $form = array('games'=>array('#tree'=>TRUE), 'users'=>array('#tree'=>TRUE));
  foreach ($games as $gid => $g_data) {

    // Save who is the home team and who is the visitor.
    $form['games'][$gid]['h_id']['#value'] = $g_data->h_id;
    $form['games'][$gid]['v_id']['#value'] = $g_data->v_id;

    // Load-up the home team info.
    $form['games'][$gid]['teams'][$g_data->h_id] = array(
      'tm_abbrev' => array('#value' => $teams[$g_data->h_id]->tm_abbrev),
      'short_name' => array('#value' => $teams[$g_data->h_id]->short_name),
      'long_name' => array('#value' => $teams[$g_data->h_id]->long_name)
    );

    // Load-up the visiting team info.
    $form['games'][$gid]['teams'][$g_data->v_id] = array(
      'tm_abbrev' => array('#value' => $teams[$g_data->v_id]->tm_abbrev),
      'short_name' => array('#value' => $teams[$g_data->v_id]->short_name),
      'long_name' => array('#value' => $teams[$g_data->v_id]->long_name)
    );

    // Load-up the game titles.
    $form['games'][$gid]['short_game_title']['#value'] =
      $form['games'][$gid]['teams'][$g_data->v_id]['tm_abbrev']['#value'] .
      ' @ ' .
      $form['games'][$gid]['teams'][$g_data->h_id]['tm_abbrev']['#value'];

    $form['games'][$gid]['long_game_title']['#value'] =
      $form['games'][$gid]['teams'][$g_data->v_id]['long_name']['#value'] .
      ' @ ' .
      $form['games'][$gid]['teams'][$g_data->h_id]['long_name']['#value'];



    // Figure out who won.
    if ( $g_data->h_score == NULL ) {
      $form['games'][$gid]['winner'] = array('#value' => 0);  // game not played
    }
    elseif ($g_data->h_score == $g_data->v_score) {
      $form['games'][$gid]['winner'] = array('#value' => -1); // game ended in tie
    }
    elseif ($g_data->h_score > $g_data->v_score) {
      $form['games'][$gid]['winner'] = array('#value' => $g_data->h_id);  // home team won.
    }
    elseif ($g_data->v_score > $g_data->h_score) {
      $form['games'][$gid]['winner'] = array('#value' => $g_data->v_id);  // visiting team won.
    }

    // Save the score (May be NULL).
    $form['games'][$gid]['teams'][$g_data->h_id]['score'] = array('#value' => $g_data->h_score);
    $form['games'][$gid]['teams'][$g_data->v_id]['score'] = array('#value' => $g_data->v_score);


  }

  $totals = get_user_totals($league, $week->wid); //sorted by weekly scores
  $picks = get_picks( array('league_id' => $league->lid, 'week_id' => $week->wid) );
  foreach ($totals as $uid => $tot) {
    $user_form = array(
      'uid' => array('#value' => $uid),
      'user_link' => array('#value' => theme('username', $tot['user_obj'])),
      'weekly_rank' => array('#value' => $tot['ranking']),
      'weekly_points' => array('#value' => $tot['total']),
      'picks' => array()
    );

    // Loop collect picks and pick points.
    foreach ($form['games'] as $gid => $game_info) {

      // Week has not yet started, don't show picks or scores unless auth user is the pick user.
      if ($week->week_past == 0) {
        if (isset ($picks[$gid][$uid]->winnerpick_id)) {
          if ( $uid == $user->uid ) {
            // User is allowed to see their own picks, but not others.
            $user_form['picks'][$gid]['picked'] = array('#value' => $picks[$gid][$uid]->winnerpick_id);
          }
          else {
            // Use 0 to mean "you are not allowed to see this."
            $user_form['picks'][$gid]['picked'] = array('#value' => 0);
          }
        }
        else {
          // Use a -1 to mean they haven't picked yet.
          // It also might mean they are using another pick system which may
          // use another field.
          $user_form['picks'][$gid]['picked'] = array('#value' => -1);
        }
      }

      // Week start has passed, go ahead and show picks and totals.
      else {
        // User is allowed to see all picks now.
        $user_form['picks'][$gid]['picked'] = array('#value' => $picks[$gid][$uid]->winnerpick_id);
        $user_form['picks'][$gid]['points'] = array('#value' => $tot['weeks'][$week->wid]['games'][$gid]);
      }
    }

    // Since we are using push they should stay in ranking order.
    array_push($form['users'], $user_form);
  }

  return $form;
}


function theme_pick_summary_form($form) {

  $header = array();
  $header[] = t('Weekly Rank');
  $header[] = t('Points');
  foreach (element_children($form['games']) as $gid) {
    $short_game_title = str_replace(" @ ", "<br/>@<br/>", $form['games'][$gid]['short_game_title']['#value']);
    $header[] = array('data' => $short_game_title, 'title' => $form['games'][$gid]['long_game_title']['#value']);
  }

  $rows = array();
  foreach (element_children($form['users']) as $key) {
    $cols = array();
    $cols[] = array('data' => $form['users'][$key]['weekly_rank']['#value'] . '. '. $form['users'][$key]['user_link']['#value'], 'class'=>'users');
    $cols[] = $form['users'][$key]['weekly_points']['#value'];

    foreach (element_children($form['games']) as $gid) {
      $class = '';
      if ( $form['users'][$key]['picks'][$gid]['points']['#value'] > 0 ) {
        $class = 'win';
      } elseif ( $form['users'][$key]['picks'][$gid]['points']['#value'] == 0 ) {
        $class = 'loss';
      }

      $cols[] = array('data' => $form['games'][$gid]['teams'][$form['users'][$key]['picks'][$gid]['picked']['#value']]['short_name']['#value'] . ": " .
        $form['users'][$key]['picks'][$gid]['points']['#value'], 'class' => $class);
    }


    array_push($rows, $cols);
  }

  return theme('table', $header, $rows, array('class'=>'pickem-pick-summary'));
}
