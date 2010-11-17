<?php
// $Id: $

/**
 * @file
 * Page callbacks for adding, editing, and deleting user picks.
 */


/**
 * Menu callback; presents the list of all weeks and games.
 */
function mypicks_all_page($league) {
  return theme('mypicks_all', $league);
}


/**
 * Display the list of all weeks and games.
 * The user would pick a week link from here to make their weekly picks.
 */
function theme_mypicks_all($league) {
  global $_PICKEM_MY_LEAGUES;
  global $user;

  // Param check: user must be a member of this league to continue.
  if ( !is_my_league($league) ) {
    return drupal_not_found();
  }

  drupal_set_title("My Picks - ". $user->name);

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l($league->name, 'pickem/'. $league->lid);
  drupal_set_breadcrumb($breadcrumb);

  $teams = get_teams(array('gameset_id' => $league->gsid));
  $games = get_games(array('league_id' => $league->lid), array('by_week' => 1, 'order_by' => 'week_past, g.gametime, g.gid'));


  $header = array(t('Week'), t('Games'));
  $rows = array();
  foreach ($games as $wid => $weeks_games) {
    $cols = array();
    $current_week = '';
    foreach ($weeks_games as $gid => $game) {
      // The first column should be the week number link.
      if ( count($cols) == 0 ) {
        $cols[] = array('data' => l('Week&nbsp;'. $game->wk_number, "pickem/$league->lid/mypicks/" . $wid, array('html' => TRUE)), 'class' => 'week');
      }
      $current_week .= '<div class="picks-float">' . $teams[$game->v_id]->short_name .'<br/>@'. $teams[$game->h_id]->short_name . '</div>';
    }
    $cols[] = $current_week;
    $rows[] = $cols;
  }
  return theme('table', $header, $rows, array('class' => 'pickem-picks'));
}


/**
 * Menu callback; presents a list of a single weeks picks.
 */
function mypicks_week_page($league, $week) {
  global $user;
  drupal_set_title($week->wk_name .' Picks - '. $user->name);
  return drupal_get_form('mypicks_week_form', $league, $week);
}


function mypicks_week_form($form_state=NULL, $league, $week) {
  global $user;
  $form = build_picks_week_form(&$form_state, $league, $week, $user);

  return $form;
}


function theme_mypicks_week_form($form) {
  global $_PICKEM_TIMEZONE;

  $league = $form['#parameters'][2];

  $header = array();
  $header[] = t('Matchup');
  $header[] = t('Time(%pickem-timezone)', array('%pickem-timezone' => $_PICKEM_TIMEZONE));

  $rows = array();
  $last = count(element_children($form['games']));
  $count = 0;
  foreach (element_children($form['games']) as $gid) {
    $count++;
    $cols = array();

    if (isset($form['games'][$gid]['pick'])) {
      $cols[] = drupal_render($form['games'][$gid]['pick']);
    }
    else {
      $cols[] = drupal_render($form['games'][$gid]['pick_v_score']) . drupal_render($form['games'][$gid]['pick_h_score']);
    }

    $cols[] = drupal_render($form['games'][$gid]['gametime']);
    $rows[] = $cols;
  }

  $output = $league->scoring_engine['description'];
  $output .= theme('table', $header, $rows);
  $output .= drupal_render($form);
  return $output;
}


function mypicks_week_form_submit($form, &$form_state) {
  global $user;

  $league = $form['#parameters'][2];
  $week = $form['#parameters'][3];

  // You should not be able to save picks after lockup.
  if ($week->week_past == 1) {
    drupal_set_title($week->wk_name .' picks are locked.');
    print theme('page', "You cannot change your picks now.");
    return;
  }

  // Check and see if the user wants to save to all their leagues that use the same gameset.
  $save_for_these_leagues = array();
  if ( $form_state['values']['apply_to_all_leagues'] == 1 ) {
    $save_for_these_leagues = get_my_leagues($league->gsid);
  }
  else {
    $save_for_these_leagues[] = $league;
  }

  foreach ($save_for_these_leagues as $l) {
    // Only save for league using the same scoring system.
    if ($l->scoring_type == $league->scoring_type) {
      drupal_set_message("Saving picks for " . $l->name);
      foreach ($form_state['values']['games'] as $gid => $pick) {
        $pick['gid'] = $gid;
        set_pick($user->uid, $l, $pick);
      }
    }
  }


}
