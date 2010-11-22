<?php
// $Id: $

/**
 * @file
 * Page callbacks for the admin picks pages.
 */


function admin_picks_router($league = NULL, $account = NULL, $week = NULL) {
  if ( is_numeric($_POST['league_select']) ) {
    drupal_goto("admin/pickem/picks/" . $_POST['league_select']);
  }
  if ( is_numeric($_POST['user_select']) ) {
    drupal_goto("admin/pickem/picks/$league->lid/" . $_POST['user_select']);
  }

  if ( is_null($league) ) {
    drupal_set_title('Choose a League to administer');
    return drupal_get_form('league_select_form');
  }
  if ( is_null($account) ) {
    drupal_set_title('Choose a User to administer');
    return drupal_get_form('user_select_form', $league);
  }
}


/**
 * Menu callback; presents the list of all weeks and games.
 */
function admin_picks_all_page($league, $account) {
  return theme('admin_picks_all_page', $league, $account);
}


/**
 * Display the list of all weeks and games.
 * The user would pick a week link from here to make their weekly picks.
 */
function theme_admin_picks_all_page($league, $account) {

  // Param check: user must be a member of this league to continue.
  if ( !is_league_member($league, $account) ) {
    return drupal_not_found();
  }

  drupal_set_title("Administer Picks/Choose Week");

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
        $cols[] = array('data' => l($game->wk_number, "admin/pickem/picks/$league->lid/$account->uid/" . $wid), 'class' => 'week');
      }
      $current_week .= '<div class="picks-float">' . $teams[$game->v_id]->short_name .'<br/>@'. $teams[$game->h_id]->short_name . '</div>';
    }
    $cols[] = $current_week;
    $rows[] = $cols;
  }

  $ouput = '';
  $output = 'League: '. $league->name .'<br/>';
  $output .= 'User: '. $account->name .'<br/>';
  $output .= theme('table', $header, $rows, array('class' => 'pickem-picks'));
  return $output;
}


/**
 * Menu callback; presents a list of a single weeks picks.
 */
function admin_picks_week_page($league, $week, $account) {
  drupal_set_title('Administer Picks');
  return drupal_get_form('admin_picks_week_form', $league, $week, $account);
}


function admin_picks_week_form(&$form_state=NULL, $league, $week, $account) {
  $form = build_picks_week_form(&$form_state, $league, $week, $account);

  // Enable the input boxes and radio boxes, this is an admin screen.
  // It doesn't matter the week may be past.
  foreach (element_children($form['games']) as $gid) {
    unset($form['games'][$gid]['pick_v_score']['#attributes']['readonly']);
    unset($form['games'][$gid]['pick_v_score']['#attributes']['disabled']);
    unset($form['games'][$gid]['pick_h_score']['#attributes']['readonly']);
    unset($form['games'][$gid]['pick_h_score']['#attributes']['disabled']);
    unset($form['games'][$gid]['pick']['#attributes']['readonly']);
    unset($form['games'][$gid]['pick']['#attributes']['disabled']);
  }

  // If $account is entered in more than one league with the same scoring type,
  // add a checkbox to apply these picks to related leagues.
  $leagues = get_my_leagues($league->gsid, $account->uid);
  $apply_to_leagues = array();
  foreach ($leagues as $l) {
    if ($l->scoring_type == $league->scoring_type) {
      $apply_to_leagues[] = $l->name;
    }
  }

  // Turn on the apply to all leagues button if needed.
  $form['apply_to_all_leagues']['#access'] = count($apply_to_leagues) > 1;

  // Turn on the submit button.
  unset($form['submit']['#access']);

  return $form;
}


function theme_admin_picks_week_form($form) {
  global $_PICKEM_TIMEZONE;

  $league = $form['#parameters'][2];
  $week = $form['#parameters'][3];
  $account = $form['#parameters'][4];


  $header = array();
  $header[] = t('Matchup');
  $header[] = t('Status');
  $header[] = t('Time(%pickem-timezone)', array('%pickem-timezone' => $_PICKEM_TIMEZONE));

  $rows = array();
  foreach (element_children($form['games']) as $gid) {
    $cols = array();

    if (isset($form['games'][$gid]['pick'])) {
      $cols[] = drupal_render($form['games'][$gid]['pick']);
    }
    else {
      $cols[] = drupal_render($form['games'][$gid]['pick_v_score']) . drupal_render($form['games'][$gid]['pick_h_score']);
    }
    $cols[] = drupal_render($form['games'][$gid]['pick_status']);
    $cols[] = drupal_render($form['games'][$gid]['gametime']);
    $rows[] = $cols;
  }

  $output = '';
  $output .= 'League: '. $league->name .'<br/>';
  $output .= 'User: '. $account->name .'<br/>';
  $output .= 'Week: '. $week->wk_name .'<br/>';

  $output .= theme('table', $header, $rows);
  $output .= drupal_render($form);
  return $output;
}


function admin_picks_week_form_submit($form, &$form_state) {

  $league = $form['#parameters'][2];
  $week = $form['#parameters'][3];
  $account = $form['#parameters'][4];

  // Check and see if the user wants to save to all their leagues that use the same gameset.
  $save_for_these_leagues = array();
  if ( $form_state['values']['apply_to_all_leagues'] == 1 ) {
    $save_for_these_leagues = get_my_leagues($league->gsid, $account->uid);
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
        set_pick($account->uid, $l, $pick, 'admin');
      }
    }
  }


}
