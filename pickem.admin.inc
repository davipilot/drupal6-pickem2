<?php
// $Id: pickem_admin.inc,v 1.1.2.3 2008/09/22 18:20:32 jvandervort Exp $

/**
 * @file
 * includes all pickem admin functions.
*/

/**
 * Menu callback which shows an overview page.
 */
function pickem_overview_page() {
  $items = pickem_menu();
  $content = '<dl class="admin-list">';
  foreach ( $items as $path => $menu ) {
    $t = explode('/', $path);
    if ( count($t) == 3 ) {
      if ( ( $t[0] == 'admin' ) && $t[1] == 'pickem'  && $menu['type'] == MENU_NORMAL_ITEM) {
        $content .= '<dt>'. l($menu['title'], $path) .'</dt>';
        if ( isset($menu['description']) ) {
          $content .= '<dd>'. $menu['description'] .'</dd>';
        }
      }
    }
  }
  $content .= "</dl>";
  print theme('page', $content);
}

function pickem_settings(&$form_state=NULL) {
  $form = array();
  $form['pickem_commisioner_email'] = array(
    '#type' => 'textfield',
    '#size' => 100,
    '#title' => t('Email Address of the Commisioner'),
    '#default_value' => variable_get('pickem_commisioner_email', ''),
    '#description' => t("This is the return address for all leaguewide pickem email."),
  );

  $form['pickem_default_email_reminders'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send email reminders'),
    '#description' => t("If this is checked, each new user will get 'Your picks are not complete' emails.  Each user can override this if they wish."),
    '#return_value' => 1,
    '#default_value' => variable_get('pickem_default_email_reminders', 1)
  );

  $timezones['US/Eastern'] = 'US/Eastern';
  $timezones['America/New_York'] = 'America/New York - Eastern';
  $timezones['America/Chicago'] = 'America/Chicago - Central';
  $timezones['America/Denver'] = 'America/Denver - Mountain';
  $timezones['America/Los_Angeles'] = 'US/America/Los Angeles - Pacific';
  $timezones['Pacific/Honolulu'] = 'Pacific/Honolulu';

  $form['pickem_time_zone'] = array(
    '#title' => t('Pickem time zone'),
    '#description' => 'This MUST MATCH all of your game sets.<br/>NOTE: NFL schedules use EDT which changes mid-season.<br/>Current Server Time='. date("r"),
    '#type' => 'select',
    '#default_value' => variable_get('pickem_time_zone', 'US/Eastern'),
    '#options' => $timezones
  );


  $form['pickem_auto_signup'] = array(
    '#type' => 'fieldset',
    '#title' => t('Auto Signup'),
    '#collapsible' => TRUE,
  );

  $form['pickem_auto_signup']['pickem_auto_signup_enable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable'),
    '#return_value' => 1,
  
    '#default_value' => variable_get('pickem_auto_signup_enable', 0),
    '#description' => t("This will automatically assign new users to the selected league."),
  );

  $all_leagues = get_leagues();
  $leagues = array();
  foreach ($all_leagues as $l) {
    $leagues[$l->lid] = $l->name;
  }

  $form['pickem_auto_signup']['pickem_auto_signup_default_league'] = array(
    '#type' => 'select',
    '#default_value' => variable_get('pickem_auto_signup_default_league', 0),
    '#options' => $leagues,
    '#title' => t('League'),
    '#description' => t("Which league should users be signed up for?"),
  );

  // Check to see if the jstimer is configured correctly.
  if (module_exists("jstimer")) {
    if (!module_exists("jst_timer")) {
      drupal_set_message(l(t('Javascript Timer widget'), 'admin/build/modules') . t(' is not enabled for the pickem module.  Make sure "Widget: timer" is enabled under the Javascript timer package or the timer will not show properly.'), 'error');
    }
    if (variable_get('jstimer_js_load_option', 0) != 1 ) {
      drupal_set_message(l(t('Javascript Timer settings'), 'admin/settings/jstimer') . t(' are set incorrectly for the pickem module.  Make sure Javascript load options are set to EVERY PAGE or the timer will not show properly.'), 'error');
    }
  }

  return system_settings_form($form);
}


/**
 * Admin Gamesets
 */
function theme_pickem_gamesets_list($form) {

  $header = array(
    t('Game Set'),
    array('data' => t('Operations'), 'colspan' => '5')
  );

  $rows = array();
  foreach (element_children($form['gamesets']) as $key) {
    $rows[] = array(
      drupal_render($form['gamesets'][$key]['gs_name']),
      drupal_render($form['gamesets'][$key]['edit_link']),
      drupal_render($form['gamesets'][$key]['teams_link']),
      drupal_render($form['gamesets'][$key]['weeks_link']),
      drupal_render($form['gamesets'][$key]['export_link']),

      drupal_render($form['gamesets'][$key]['delete_link'])
    );
  }

  $output .= theme('table', $header, $rows);

  return $output;
}
function pickem_gamesets_list(&$form_state=NULL) {
  //drupal_set_message("admin_leagues_form");

  $form = array(
    '#theme' => 'pickem_gamesets_list',
    '#tree' => TRUE,
  );

  $gamesets = get_gamesets();
  foreach ($gamesets as $gsid => $gameset) {
    $form['gamesets'][$gsid] = array(
      'gs_name' => array(
        '#value' => $gameset->gs_name
      ),
      'weeks_link' => array(
        '#value' => l('weeks', 'admin/pickem/weeks/'. $gsid .'/list')
      ),
      'teams_link' => array(
        '#value' => l('teams', 'admin/pickem/teams/'. $gsid .'/list')
      ),
      'edit_link' => array(
        '#value' => l('edit game set', 'admin/pickem/gamesets/edit/'. $gsid)
      ),
      'delete_link' => array(
        '#value' => l('delete game set', 'admin/pickem/gamesets/delete/'. $gsid)
      ),
      'export_link' => array(
        '#value' => l('export', 'admin/pickem/gamesets/export/'. $gsid)
      ),
    );
  }

  return $form;
}


/**
 * LEAGUES
 * Menu callback; Build the pickem league editing form.
 */
function pickem_gameset_edit(&$form_state, $type, $gameset) {

  if ( is_null($gameset) ) {
    // this should match the table field names
    $gameset = new stdClass();
    $gameset->gsid = 0;
    $gameset->gs_name = '';
  }

  $form = array();
  $form['gameset']['gsid'] = array(
    '#title'  => t('Game Set ID'),
    '#type' => 'hidden',
    '#size' => 20,
    '#maxlength' => 50,
    '#default_value' => $gameset->gsid
  );
  $form['gameset']['gs_name'] = array(
    '#title'  => t('Name'),
    '#required' => TRUE,
    '#type' => 'textfield',
    '#size' => 50,
    '#default_value' => $gameset->gs_name
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}
/**
 * Validate form values for a league being added or edited.
 */
function pickem_gameset_edit_validate($form, &$form_state) {
  $gameset = &$form_state['values'];
  // not used since it is required, keep this for a while.
  if (empty($gameset['gs_name'])) {
    form_set_error('gs_name', t('You must at least choose a name.'));
  }
}
/**
 * Submit form values for a league being added or edited.
 */
function pickem_gameset_edit_submit($form, &$form_state) {
  $gameset = $form_state['values'];

  if ( $gameset['gsid'] == 0 ) {
    $query = "insert into {pickem_gamesets} (gs_name) values('%s')";
    $result = db_query($query, $gameset['gs_name']);

    $t_string = t('Added game set %name.', array('%name' => $gameset['gs_name']));
    drupal_set_message($t_string);
    watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);
  }
  else {
    $query = "update {pickem_gamesets} set gs_name='%s' where gsid=%d";
    $result = db_query($query, $gameset['gs_name'], $gameset['gsid']);

    $t_string = t('Updated game set %name.', array('%name' => $gameset['gs_name']));
    drupal_set_message($t_string);
    watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);
  }

  $form_state['redirect'] = 'admin/pickem/gamesets';
}
function pickem_gameset_delete_confirm(&$form_state, $gameset) {
  $form['#gameset'] = $gameset;
  $caption = '';
  $caption .= '<p>'. t('This action cannot be undone.  All related teams, weeks, and games will be deleted.') .'</p>';
  return confirm_form($form, t('Are you sure you want to delete the game set %name?', array('%name' => $gameset->gs_name)), 'admin/pickem/gamesets', $caption, t('Delete'));
}
function pickem_gameset_delete_confirm_submit($form, &$form_state) {
  $gameset = $form['#gameset'];
  db_query("DELETE FROM {pickem_gamesets} WHERE gsid = %d", $gameset->gsid);
  db_query("DELETE FROM {pickem_teams} WHERE gsid = %d", $gameset->gsid);
  db_query("DELETE FROM {pickem_weeks} WHERE gsid = %d", $gameset->gsid);
  db_query("DELETE FROM {pickem_games} WHERE gsid = %d", $gameset->gsid);

  $t_string = t('Deleted game set %name.', array('%name' => $gameset->gs_name));
  drupal_set_message($t_string);
  watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);

  $form_state['redirect'] = 'admin/pickem/gamesets';
}


/**
 * Admin Teams
 */
function theme_pickem_teams_list($form) {
  $header = array(
    t('Team'),
    t('Conference'),
    array('data' => t('Operations'), 'colspan' => '3')
  );

  $rows = array();
  foreach (element_children($form['teams']) as $key) {
    $rows[] = array(
      drupal_render($form['teams'][$key]['long_name']),
      drupal_render($form['teams'][$key]['conference']),
      drupal_render($form['teams'][$key]['edit_link']),
      drupal_render($form['teams'][$key]['delete_link']),
    );
  }

  $output = theme('table', $header, $rows);

  return $output;
}
function pickem_teams_list($form_state=NULL, $gameset) {

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l($gameset->gs_name, 'admin/pickem/gamesets');
  drupal_set_breadcrumb($breadcrumb);

  $form = array(
    '#theme' => 'pickem_teams_list',
    '#tree' => TRUE,
  );

  $teams = get_teams(array('gameset_id' => $gameset->gsid));
  foreach ($teams as $tid => $team) {
    $form['teams'][$tid] = array(
      'long_name' => array(
        '#value' => $team->long_name
      ),
      'conference' => array(
        '#value' => $team->conference
      ),
      'edit_link' => array(
        '#value' => l('edit', 'admin/pickem/teams/'. $gameset->gsid .'/edit/'. $tid)
      ),
      'delete_link' => array(
        '#value' => l('delete', 'admin/pickem/teams/'. $gameset->gsid .'/delete/'. $tid)
      ),
    );
  }

  return $form;
}
function pickem_team_edit(&$form_state, $type, $gameset, $team) {

  if ( is_null($team) ) {
    // this should match the table field names
    $team = new stdClass();
    $team->tid = 0;
    $team->gsid = $gameset->gsid;
    $team->tm_city = '';
    $team->tm_abbrev = '';
    $team->tm_nick = '';
    $team->tm_name = '';
    $team->conference = '';
    $team->division = '';
  }

  $form = array();
  $form['team']['tid'] = array(
      '#title'  => t('Team ID'),
      '#type' => 'hidden',
      '#size' => 20,
      '#maxlength' => 50,
      '#default_value' => $team->tid
  );
  $form['team']['gsid'] = array(
      '#title'  => t('Game Set ID'),
      '#type' => 'hidden',
      '#size' => 20,
      '#maxlength' => 50,
      '#default_value' => $team->gsid
  );

  $form['team']['tm_city'] = array(
      '#title'  => t('City'),
      '#type' => 'textfield',
      '#size' => 30,
      '#maxlength' => 50,
      '#default_value' => $team->tm_city
  );
  $form['team']['tm_abbrev'] = array(
    '#title'  => t('Abbreviation(5 max)'),
    '#type' => 'textfield',
    '#size' => 5,
    '#maxlength' => 5,
    '#default_value' => $team->tm_abbrev
  );
  $form['team']['tm_nick'] = array(
    '#title'  => t('Nickname'),
    '#type' => 'textfield',
    '#size' => 30,
    '#maxlength' => 50,
    '#default_value' => $team->tm_nick
  );
  $form['team']['tm_name'] = array(
    '#title'  => t('Name'),
    '#type' => 'textfield',
    '#size' => 30,
    '#maxlength' => 50,
    '#default_value' => $team->tm_name
  );
  $form['team']['conference'] = array(
    '#title'  => t('Conference'),
    '#type' => 'textfield',
    '#size' => 30,
    '#maxlength' => 50,
    '#default_value' => $team->conference
  );
  $form['team']['division'] = array(
    '#title'  => t('Division'),
    '#type' => 'textfield',
    '#size' => 30,
    '#maxlength' => 50,
    '#default_value' => $team->division
  );
  $form['team']['logo_small'] = array(
    '#title'  => t('Small Logo Filename'),
    '#type' => 'textfield',
    '#size' => 50,
    '#maxlength' => 128,
    '#default_value' => $team->logo_small
  );



  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}
/**
 * Validate form values for a menu link being added or edited.
 */
function pickem_team_edit_validate($form, &$form_state) {
  $team = &$form_state['values'];
  //if ($team['tm_name'] == 'lions') {
  //  form_set_error('tm_name', t("Sorry for you.", array('@tm_name' => $team['tm_name'])));
  //}
}
function pickem_team_edit_submit($form, &$form_state) {
  $team = $form_state['values'];

  if ( $team['tid'] == 0 ) {
    $query = "insert into {pickem_teams} (gsid, tm_city, tm_abbrev, tm_name, tm_nick, conference, division, logo_small) values(%d, '%s','%s','%s','%s','%s','%s', '%s')";
    $result = db_query($query, $team['gsid'], $team['tm_city'], $team['tm_abbrev'], $team['tm_name'], $team['tm_nick'], $team['conference'], $team['division'], $team['logo_small'] );

    $t_string = t('Added team %name.', array('%name' => $team['tm_name']));
    drupal_set_message($t_string);
    watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);
  }
  else {
    $query = "update {pickem_teams} set gsid=%d, tm_city='%s', tm_abbrev='%s', tm_name='%s', tm_nick='%s', conference='%s', division='%s', logo_small='%s' where tid=%d";
    $result = db_query($query, $team['gsid'], $team['tm_city'], $team['tm_abbrev'], $team['tm_name'], $team['tm_nick'], $team['conference'], $team['division'], $team['logo_small'], $team['tid']);

    $t_string = t('Updated team %name.', array('%name' => $team['tm_name']));
    drupal_set_message($t_string);
    watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);
  }

  $form_state['redirect'] = 'admin/pickem/teams/'. $team['gsid'];
}
/**
 * Build a confirm form for deletion of a team.
 */
function pickem_team_delete_confirm(&$form_state, $gameset, $team) {
  $form['#team'] = $team;
  $caption = '';
  $caption .= '<p>'. t('This action cannot be undone.') .'</p>';
  return confirm_form($form, t('Are you sure you want to delete the team %name from %gameset?', array('%name' => $team->tm_city .' '. $team->tm_name, '%gameset' => $gameset->gs_name)), 'admin/pickem/teams', $caption, t('Delete'));
}
/**
 * Delete the team.
 */
function pickem_team_delete_confirm_submit($form, &$form_state) {
  $team = $form['#team'];
  db_query("DELETE FROM {pickem_teams} WHERE tid = %d", $team->tid);

  $t_string = t('Deleted team %name.', array('%name' => $team->tm_city .' '. $team->tm_name));
  drupal_set_message($t_string);
  watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);

  $form_state['redirect'] = 'admin/pickem/teams/'. $team->gsid;
}


/**
 * Weeks
 */
function theme_pickem_weeks_list($form) {

  $header = array(
    t('Week'),
    t('Name'),
    t('First Game Time'),
    array('data' => t('Operations'), 'colspan' => '3')
  );
  $rows = array();
  foreach (element_children($form['weeks']) as $key) {
    $rows[] = array(
      drupal_render($form['weeks'][$key]['number']),
      drupal_render($form['weeks'][$key]['name']),
      drupal_render($form['weeks'][$key]['firstgame']),
      drupal_render($form['weeks'][$key]['edit_link']),
      drupal_render($form['weeks'][$key]['list_games_link']),
      drupal_render($form['weeks'][$key]['delete_link']),
    );
  }
  $output = theme('table', $header, $rows);

  return $output;
}
function pickem_weeks_list($form_state=NULL, $gameset) {

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l($gameset->gs_name, 'admin/pickem/gamesets');
  drupal_set_breadcrumb($breadcrumb);

  $form = array(
    '#theme' => 'pickem_weeks_list',
    '#tree' => TRUE,
  );

  $weeks = get_weeks(array('gameset_id' => $gameset->gsid));

  foreach ( $weeks as $wid => $week ) {
    $form['weeks'][] = array(
      'number' => array(
        '#value' => $week->wk_number,
      ),
      'name' => array(
        '#value' => $week->wk_name,
      ),
      'firstgame' => array(
        '#value' => $week->wk_firstgame,
      ),
      'edit_link' => array(
        '#value' => l('edit week', 'admin/pickem/weeks/'. $gameset->gsid .'/edit/'. $wid)
      ),
      'list_games_link' => array(
        '#value' => l('games', 'admin/pickem/games/'. $wid)
      ),
      'delete_link' => array(
        '#value' => l('delete week', 'admin/pickem/weeks/'. $gameset->gsid .'/delete/'. $wid)
      ),
    );
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save Weeks')
  );
  return $form;
}
/**
 * Menu callback; Build the pickem single team editing form.
 */
function pickem_week_edit(&$form_state, $type, $gameset, $week) {

  if ( is_null($week) ) {
    // this should match the table field names
    $week = new stdClass();
    $week->wid = 0;
    $week->gsid = $gameset->gsid;
    $week->wk_number = 0;
    $week->wk_name = '';
    $week->wk_abbrev = '';
    $week->wk_firstgame = '';
    $week->wk_points = 1;
    $week->playoffs = 0;
  }

  $form = array();
  $form['week'] = array(
    'wid' => array(
      '#title' => 'Week ID',
      '#type' => 'hidden',
      '#default_value' => $week->wid,
    ),
    'gsid' => array(
      '#title' => 'Game Set ID',
      '#type' => 'hidden',
      '#default_value' => $gameset->gsid,
    ),

    'wk_number' => array(
      '#title' => 'Week Number',
      '#type' => 'textfield',
      '#size' => 3,
      '#maxlength' => 3,
      '#default_value' => $week->wk_number,
    ),
    'wk_name' => array(
      '#title' => 'Name',
      '#type' => 'textfield',
      '#size' => 30,
      '#maxlength' => 45,
      '#default_value' => $week->wk_name,
    ),
    'wk_abbrev' => array(
      '#title' => 'Abbrev',
      '#type' => 'textfield',
      '#size' => 10,
      '#maxlength' => 15,
      '#default_value' => $week->wk_abbrev,
    ),
    'wk_firstgame' => array(
      '#title' => 'First Game Time',
      '#type' => 'textfield',
      '#size' => 30,
      '#maxlength' => 35,
      '#default_value' => $week->wk_firstgame,
    ),
    'wk_points' => array(
      '#title' => 'Points',
      '#type' => 'textfield',
      '#size' => 5,
      '#maxlength' => 5,
      '#default_value' => $week->wk_points,
    ),
    'playoffs' => array(
      '#title' => 'Week Type',
      '#type' => 'radios',
      '#default_value' => $week->playoffs,
      '#options' => array( 0 => t('Regular Season'), 1 => t('Playoffs') )
    )
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}
function pickem_week_edit_validate($form, &$form_state) {
  $week = &$form_state['values'];
}
function pickem_week_edit_submit($form, &$form_state) {
  $week = $form_state['values'];

  if ( $week['wid'] == 0 ) {
    $query = "insert into {pickem_weeks} (gsid, wk_number, wk_name, wk_abbrev, wk_firstgame, wk_points, playoffs) values(%d, %d, '%s', '%s', '%s', %d, %d)";
    $result = db_query($query, $week['gsid'], $week['wk_number'], $week['wk_name'], $week['wk_abbrev'], date($week['wk_firstgame']), $week['wk_points'], $week['playoffs']);

    $t_string = t('Added week %name.', array('%name' => $week['wk_name']));
    drupal_set_message($t_string);
    watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);
  }
  else {
    $query = "update {pickem_weeks} set gsid=%d, wk_number=%d, wk_name='%s', wk_abbrev='%s', wk_firstgame='%s', wk_points=%d, playoffs=%d where wid=%d";
    $result = db_query($query, $week['gsid'], $week['wk_number'], $week['wk_name'], $week['wk_abbrev'], date($week['wk_firstgame']), $week['wk_points'], $week['playoffs'], $week['wid']);

    $t_string = t('Updated week %name.', array('%name' => $week['wk_name']));
    drupal_set_message($t_string);
    watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);
  }

  cache_clear_all('pickem_', 'cache', TRUE);
  $form_state['redirect'] = 'admin/pickem/weeks/'. $week['gsid'];
}
function pickem_week_delete_confirm(&$form_state, $gameset, $week) {
  $form['#week'] = $week;
  $caption = '';
  $caption .= '<p>'. t('This action cannot be undone.') .'</p>';
  return confirm_form($form, t('Are you sure you want to delete the week %name?', array('%name' => $week->wk_name)), 'admin/pickem/weeks', $caption, t('Delete'));
}
function pickem_week_delete_confirm_submit($form, &$form_state) {
  $week = $form['#week'];
  db_query("DELETE FROM {pickem_weeks} WHERE wid = %d", $week->wid);

  $t_string = t('Deleted week %name.', array('%name' => $week->wk_name));
  drupal_set_message($t_string);
  watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);

  $form_state['redirect'] = 'admin/pickem/weeks';
}

/*
  Admin Games
*/
function theme_pickem_games_list($form) {
  $header = array(
    t('Matchup'),
    array('data' => t('Operations'), 'colspan' => '3')
  );
  $rows = array();
  foreach (element_children($form['games']) as $key) {
    $rows[] = array(
      drupal_render($form['games'][$key]['matchup']),
      drupal_render($form['games'][$key]['edit_link']),
      drupal_render($form['games'][$key]['delete_link']),
    );
  }
  $output = theme('table', $header, $rows);
  return $output;
}
function pickem_games_list(&$form_state=NULL, $week) {

  $gameset = gameset_load($week->gsid);

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l($gameset->gs_name, 'admin/pickem/gamesets');
  $breadcrumb[] = l($week->wk_name, 'admin/pickem/weeks/'. $week->gsid .'/list');
  drupal_set_breadcrumb($breadcrumb);

  $form = array(
    '#theme' => 'pickem_games_list',
    '#tree' => TRUE,
  );

  $teams = get_teams();
  $games = get_games(array('week_id' => $week->wid));
  foreach ($games as $gid => $game) {
    $home_team = $teams[$game->h_id]->long_name;
    $vis_team = $teams[$game->v_id]->long_name;
    $matchup = "$vis_team @ $home_team (". $game->gametime .")";

    $form['games'][] = array(
      'matchup' => array(
        '#value' => $matchup
      ),
      'edit_link' => array(
        '#value' => l('edit', 'admin/pickem/games/'. $week->wid .'/edit/'. $gid)
      ),
      'delete_link' => array(
        '#value' => l('delete', 'admin/pickem/games/'. $week->wid .'/delete/'. $gid)
      ),
    );
  }

  return $form;
}
function pickem_game_edit(&$form_state, $type, $week, $game) {
  if ( is_null($game) ) {
    // this should match the table field names
    $game = new stdClass();
    $game->gid = 0;
    $game->wid = $week->wid;
    $game->gametime = $week->wk_firstgame;
    $game->h_id = '';
    $game->v_id = '';
    $game->h_spread = '';
    $game->v_spread = '';
    $game->h_score = '';
    $game->v_score = '';
  }

  $teams = get_teams(array(gameset_id => $week->gsid));
  $team_list = array();
  $team_list[0] = "";
  foreach ($teams as $t_id => $team_data) {
    $team_list[$t_id] = ($team_data->tm_abbrev <> '')?$team_data->tm_abbrev:$team_data->tm_name;
  }

  $form = array();
  $form['game']['gid'] = array(
    '#title' => t('Game ID'),
    '#type' => 'hidden',
    '#value' => $game->gid
  );
  $form['game']['wid'] = array(
    '#title' => t('Week ID'),
    '#type' => 'hidden',
    '#value' => $game->wid,
  );
  $form['game']['game_time'] = array(
    '#title' => t('Game Time'),
    '#type' => 'textfield',
    '#size' => 30,
    '#maxlength' => 45,
    '#default_value' => $game->gametime,
  );
  $form['game']['visitor_team'] = array(
    '#title' => t('Visitor Team'),
    '#type' => 'select',
    '#default_value' => $game->v_id,
    '#options' => $team_list,
    '#required' => TRUE,
  );
  $form['game']['v_spread'] = array(
    '#title' => t('Visitor Spread'),
    '#type' => 'textfield',
    '#size' => 4,
    '#maxlength' => 10,
    '#default_value' => $game->v_spread,
  );
  $form['game']['v_score'] = array(
    '#title' => t('Visitor Score'),
    '#type' => 'textfield',
    '#size' => 4,
    '#maxlength' => 10,
    '#default_value' => $game->v_score,
  );
  $form['game']['home_team'] = array(
    '#title' => t('Home Team'),
    '#type' => 'select',
    '#default_value' => $game->h_id,
    '#options' => $team_list,
    '#required' => TRUE,
  );
  $form['game']['h_spread'] = array(
    '#title' => t('Home Spread'),
    '#type' => 'textfield',
    '#size' => 4,
    '#maxlength' => 10,
    '#default_value' => $game->h_spread,
  );
  $form['game']['h_score'] = array(
    '#title' => t('Home Score'),
    '#type' => 'textfield',
    '#size' => 4,
    '#maxlength' => 10,
    '#default_value' => $game->h_score,
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}
/**
 * Validate form values for a game being added or edited.
 */
function pickem_game_edit_validate($form, &$form_state) {
  $game = &$form_state['values'];

  if ( $game['v_spread'] != -$game['h_spread']) {
    form_set_error('h_spread', t('The spread fields must be equal and opposite (ie 3 and -3).'));
  }
}
function pickem_game_edit_submit($form, &$form_state) {
  $game = $form_state['values'];

  // clear pickem_ caches, since you are usually updating scores here.
  cache_clear_all('pickem_', 'cache', TRUE);

  // add new game
  if ( $game['gid'] == 0 ) {

    // drupal bug #245335, can't insert nulls into a decimel field with db_query replacement
    $game['v_score'] = $game['v_score'] == '' ? 'NULL' : (int) $game['v_score'];
    $game['h_score'] = $game['h_score'] == '' ? 'NULL' : (int) $game['h_score'];

    $query = "insert into {pickem_games}(wid, gametime, h_id, h_spread, v_id, v_spread, h_score, v_score) values(%d, '%s', %d, %d, %d, %d, %s, %s)";
    $result = db_query($query, $game['wid'], date($game['game_time']), $game['home_team'], $game['h_spread'], $game['visitor_team'], $game['v_spread'], $game['h_score'], $game['v_score']);

    $t_string = t('Added game id %game-id.', array('%game-id' => db_last_insert_id('pickem_games', 'gid')));
    drupal_set_message($t_string);
    watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);
  }
  // update existing game
  else {

    // drupal bug #245335, can't insert nulls into a decimel field with db_query replacement
    $game['v_score'] = $game['v_score'] == '' ? 'NULL' : (int) $game['v_score'];
    $game['h_score'] = $game['h_score'] == '' ? 'NULL' : (int) $game['h_score'];

    $query = "update {pickem_games} set wid=%d, gametime='%s', h_id=%d, h_spread=%d, v_id=%d, v_spread=%d, h_score=%s, v_score=%s where gid=%d";
    $result = db_query($query, $game['wid'], date($game['game_time']), $game['home_team'], $game['h_spread'], $game['visitor_team'], $game['v_spread'], $game['h_score'], $game['v_score'], $game['gid']);

    $t_string = t('Updated game id %game-id.', array('%game-id' => $game['gid']));
    drupal_set_message($t_string);
    watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);
  }
  //drupal_goto('admin/pickem/games', 'page='. ($game['wid'] - 1));
  $form_state['redirect'] = array('admin/pickem/games/'. $game['wid']);
}
/**
 * Build a confirm form for deletion of a game.
 */
function pickem_game_delete_confirm(&$form_state, $week, $game) {
  $form['#game'] = $game;
  $caption = '';
  $caption .= '<p>'. t('This action cannot be undone.') .'</p>';

  //return confirm_form($form, t('Are you sure you want to delete the team %name?', array('%name' => $league->name)), 'admin/pickem/leagues', $caption, t('Delete'));

  return confirm_form($form, t('Are you sure you want to delete this game?'), 'admin/pickem/games/'. $week->wid, $caption, t('Delete'));
}
/**
 * Delete the game.
 */
function pickem_game_delete_confirm_submit($form, &$form_state) {
  $game = $form['#game'];
  db_query("DELETE FROM {pickem_games} WHERE gid = %d", $game->gid);

  $t_string = t('Deleted game id %gameid.', array('%gameid' => $game->gid));
  drupal_set_message($t_string);
  watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);

  $form_state['redirect'] = array('admin/pickem/games/'. $game->wid);
}


/**
 * Admin Leagues
 */
function theme_pickem_leagues_list($form) {

  $header = array(
    t('League'),
    t('Game Set'),
    array('data' => t('Operations'), 'colspan' => '4')
  );

  $rows = array();
  foreach (element_children($form['leagues']) as $key) {
    $rows[] = array(
      drupal_render($form['leagues'][$key]['name']),
      drupal_render($form['leagues'][$key]['gs_name']),
      drupal_render($form['leagues'][$key]['edit_link']),
      drupal_render($form['leagues'][$key]['list_users_link']),
      drupal_render($form['leagues'][$key]['enter_scores_link']),
      drupal_render($form['leagues'][$key]['delete_link'])
    );
  }

  $output .= theme('table', $header, $rows);

  return $output;
}
function pickem_leagues_list(&$form_state=NULL) {
  //drupal_set_message("admin_leagues_form");

  $form = array(
    '#theme' => 'pickem_leagues_list',
    '#tree' => TRUE,
  );

  $gamesets = get_gamesets();

  $query = "SELECT * from {pickem_leagues} order by lid";
  $result = db_query($query);

  while ($row = db_fetch_object($result)) {
    $form['leagues'][$row->lid] = array(
      'name' => array(
        '#value' => $row->name
      ),
      'gs_name' => array(
        '#value' => $gamesets[$row->gsid]->gs_name
      ),
      'edit_link' => array(
        '#value' => l('edit league', 'admin/pickem/leagues/edit/'. $row->lid)
      ),
      'list_users_link' => array(
        '#value' => l('users', 'admin/pickem/users/'. $row->lid)
      ),
      'enter_scores_link' => array(
        '#value' => l('scores', 'admin/pickem/scores/'. $row->lid)
      ),
      'delete_link' => array(
        '#value' => l('delete league', 'admin/pickem/leagues/delete/'. $row->lid)
      ),
    );
  }

  return $form;
}


/**
 * LEAGUES
 * Menu callback; Build the pickem league editing form.
 */
function pickem_league_edit(&$form_state, $type, $league) {

  if ( is_null($league) ) {
    // this should match the table field names
    $league = new stdClass();
    $league->lid = 0;
    $league->name = '';
    $league->forum_id = 0;
    $league->rules_node_id = '';
    $league->scoring_type = 0;
    $league->gsid = 0;
    $league->partial_league = 0;
  }
  $league_forums_options = league_forums();
  $league_gameset_options = gameset_options();

  $form = array();
  $form['league']['lid'] = array(
    '#title'  => t('League ID'),
    '#type' => 'hidden',
    '#size' => 20,
    '#maxlength' => 50,
    '#default_value' => $league->lid
  );
  $form['league']['name'] = array(
    '#title'  => t('Name'),
    '#required' => TRUE,
    '#type' => 'textfield',
    '#size' => 50,
    '#default_value' => $league->name
  );
  $form['league']['gsid'] = array(
    '#title'  => t('Game Set'),
    '#required' => TRUE,
    '#type' => 'select',
    '#default_value' => $league->gsid,
    '#options' => $league_gameset_options,
  );
  $form['league']['forum_id'] = array(
    '#title'  => t('Forum ID'),
    '#type' => 'select',
    '#default_value' => $league->forum_id,
    '#options' => $league_forums_options,
   );
  $form['league']['rules_node_id'] = array(
    '#title'  => t('Rules Node ID'),
    '#type' => 'textfield',
    '#size' => 5,
    '#default_value' => $league->rules_node_id
   );
  $form['league']['scoring_type'] = array(
    '#title'  => t('Scoring Type'),
    '#type' => 'radios',
    '#default_value' => $league->scoring_type,
    '#options' => array(PICKEM_TYPE_WIN_LOSS => t('Pick the winner'), PICKEM_TYPE_SPREAD => t('Pick the winner with point spread'), PICKEM_TYPE_SCORE_DIFF => t('Score differential'), PICKEM_TYPE_WIN_LOSS_TIEBREAK => t('Pick the winner with tie-breaker')),
    '#description' =>
      t('Pick the winner: a simple you pick the winner game.') . '<br/>' .
      t('Pick the winner with point spread: you pick the winner based on point spread.') . '<br/>' .
      t('Score differential: you pick the score of the game and get points for correct score differential and winner.') . '<br/>' .
      t('Pick the winner with tie-breaker: a simple you pick the winner game, but you pick the score for the last game to break ties.') 
  );
  $form['league']['partial_league'] = array(
    '#type' => 'checkbox',
    '#title' => t('Partial league support'),
    '#return_value' => 1,
    '#default_value' => $league->partial_league,
    '#description' => t("If you don't pick every game for every team, use this setting.  It will disable the screens that compare team's records (which will be incorrect for cherry-picked leagues.)"),
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}
function pickem_league_edit_validate($form, &$form_state) {
  $league = &$form_state['values'];
  // not used since it is required, keep this for a while.
  if (empty($league['name'])) {
    form_set_error('name', t('You must at least choose a name.'));
  }
}
function pickem_league_edit_submit($form, &$form_state) {
  $league = $form_state['values'];

  if ( $league['lid'] == 0 ) {
    $query = "insert into {pickem_leagues} (name, rules_node_id, forum_id, scoring_type, gsid, partial_league) values('%s', %d, %d, %d, %d, %d)";
    $result = db_query($query, $league['name'], $league['rules_node_id'], $league['forum_id'], $league['scoring_type'], $league['gsid'], $league['partial_league']);

    $t_string = t('Added league %league-name.', array('%league-name' => $league['name']));
    drupal_set_message($t_string);
    watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);
  }
  else {
    $query = "update {pickem_leagues} set name='%s', rules_node_id=%d, forum_id=%d, scoring_type=%d, gsid=%d, partial_league=%d where lid=%d";
    $result = db_query($query, $league['name'], $league['rules_node_id'], $league['forum_id'], $league['scoring_type'], $league['gsid'], $league['partial_league'], $league['lid']);

    $t_string = t('Updated league %league-name.', array('%league-name' => $league['name']));
    drupal_set_message($t_string);
    watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);
  }

  $form_state['redirect'] = 'admin/pickem/leagues';
}
function pickem_league_delete_confirm(&$form_state, $league) {
  $form['#league'] = $league;
  $caption = '';
  $caption .= '<p>'. t('This action cannot be undone.') .'</p>';
  return confirm_form($form, t('Are you sure you want to delete the team %name?', array('%name' => $league->name)), 'admin/pickem/leagues', $caption, t('Delete'));
}
function pickem_league_delete_confirm_submit($form, &$form_state) {
  $league = $form['#league'];
  db_query("DELETE FROM {pickem_leagues} WHERE lid = %d", $league->lid);

  $t_string = t('Deleted league %name.', array('%name' => $league->name));
  drupal_set_message($t_string);
  watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);

  $form_state['redirect'] = 'admin/pickem/leagues';
}

















function theme_pickem_users_list($form) {
  $header = array(
    t('User'),
    array('data' => t('Operations'), 'colspan' => '3')
  );

  $rows = array();
  foreach (element_children($form['users']) as $key) {
    $rows[] = array(
      drupal_render($form['users'][$key]['user_link']),
      drupal_render($form['users'][$key]['remove_link'])
    );
  }

  $output .= theme('table', $header, $rows);

  return $output;
}
function pickem_users_list($form_state=NULL, $league) {
  $form = array(
    '#theme' => 'pickem_users_list',
    '#tree' => TRUE,
  );
  $users = get_users($league);
  foreach ( $users as $user ) {
    $form['users'][] = array(
      'user_link' => array(
        '#value' => l($user->name, 'user/'. $user->uid)
      ),
      'remove_link' => array(
        '#value' => l('Remove', 'admin/pickem/users/'. $league->lid .'/remove/'. $user->uid)
      ),
    );
  }
  return $form;
}
function pickem_users_add(&$form_state, $league) {
  $form = user_select_form($form_state, $league, 'Add User', 1);
  $form['#league'] = $league;
  $form['submit'] = array('#type' => 'submit', '#value' => t('Add User'));
  unset($form['op']);  // may change this later
  return $form;
}
function pickem_users_add_validate($form, &$form_state) {
  //$user = &$form_state['values'];
  //if ($team['tm_name'] == 'lions') {
  //  form_set_error('tm_name', t("Sorry for you.", array('@tm_name' => $team['tm_name'])));
  //}
}
function pickem_users_add_submit($form, &$form_state) {
  $league = $form['#league'];
  $user_id = $form_state['values']['user_select'];
  $user = user_load(array('uid' => $user_id));

  db_query("insert into {pickem_users} (lid,uid) values (%d,%d)", $league->lid, $user_id);

  $t_string = t('"%name" was added to league "%league_name".', array('%name' => $user->name, '%league_name' => $league->name));
  drupal_set_message($t_string);
  watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);

  $form_state['redirect'] = 'admin/pickem/users/'. $league->lid;
}
function pickem_users_remove_confirm(&$form_state, $league, $user) {
  $form['#user'] = $user;
  $form['#league'] = $league;
  $caption = '';
  $caption .= '<p>'. t('This action cannot be undone.') .'</p>';
  return confirm_form($form, t('Are you sure you want to remove "%name" from league "%league_name"?', array('%name' => $user->name, '%league_name' => $league->name)), 'admin/pickem/users/'. $league->lid, $caption, t('Delete'));
}
function pickem_users_remove_confirm_submit($form, &$form_state) {
  $user = $form['#user'];
  $league = $form['#league'];
  db_query("delete from {pickem_users} where lid=%d and uid=%d", $league->lid, $user->uid);

  $t_string = t('"%name" was removed from league "%league_name".', array('%name' => $user->name, '%league_name' => $league->name));
  drupal_set_message($t_string);
  watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);

  $form_state['redirect'] = 'admin/pickem/users/'. $league->lid;
}


function theme_pickem_scores_list($form) {
  $rows = array();
  foreach (element_children($form['scores']) as $key) {
    $rows[] = array(
      array('data' => drupal_render($form['scores'][$key]['week_link'])),
      array('data' => drupal_render($form['scores'][$key]['weeks_games']))
    );
  }

  $header = array(
    t('Week'),
    t('Matchups')
  );

  $output .= '<div class="pickem">';
  $output .= theme('table', $header, $rows);
  $output .= '</div>';

  return $output;
}


function pickem_scores_list($form_state=NULL, $league) {

  $form = array(
    '#theme' => 'pickem_scores_list',
    '#tree' => TRUE,
  );

  //global $debug;
  //$debug=1;

  $games = get_games(array('league_id' => $league->lid), array('by_week' => 1));
  $teams = get_teams(array('gameset_id' => $league->gsid));

  foreach ( $games as $wid => $weeks_games ) {
    $game_str = '';
    foreach ( $weeks_games as $gid => $game ) {
      $v_score = $game->v_score <> ''?$game->v_score:'&nbsp;';
      $h_score = $game->h_score <> ''?$game->h_score:'&nbsp;';
      // need a more elegant way to apply class= in theme_ function for each game, ie not the whole week
      $game_str .= '<div class="game-list-horiz">'. $teams[$game->v_id]->short_name ." $v_score<br/>". $teams[$game->h_id]->short_name ." $h_score</div>";
    }
    $form['scores'][$wid] = array(
      'week_link' => array(
        '#value' => l($game->wk_number, 'admin/pickem/scores/'. $league->lid .'/edit/'. $wid)
      ),
      'weeks_games' => array(
        '#value' => $game_str,
      )
    );
  }
  return $form;
}


/**
 * implementation of hook_mail
 */
function pickem_mail($key, &$message, $params) {
  switch ($key) {
    case 'notice':
      $message['headers'] = $params['headers'];
      $message['subject'] = $params['subject'];
      $message['body'] = $params['body'];
      break;
  }
}


function page_admin_email_league() {
  $output = drupal_get_form('email_league_form');

  return $output;
}



function email_league_form() {
  global $user;

  $all_leagues = get_leagues();
  $leagues = array();
  foreach ($all_leagues as $l) {
    $leagues[$l->lid] = $l->name;
  }

  if (count($all_leagues) > 0) {
    $form['#attributes'] = array(
      'enctype' => "multipart/form-data"
    );
    $form['#token'] = $user->name . $user->mail;
    $form['contact_information'] = array(
      '#value' => filter_xss_admin(t('Send an e-mail message using the contact form below.'))
    );

    $form['subject'] = array(
      '#type'      => 'textfield',
      '#title'     => t('Subject'),
      '#maxlength' => 255,
      '#required'  => TRUE,
    );

    // If there is more than one category available and no default category has been selected,
    // prepend a default placeholder value.
    $form['league_id'] = array(
      '#type'          => 'select',
      '#title'         => t('Leagues'),
      '#options'       => $leagues,
      '#required'      => TRUE,
      '#multiple'      => TRUE,
    );

    $form['message'] = array(
      '#type'     => 'textarea',
      '#title'    => t('Message'),
      '#required' => TRUE,
    );
    $form['bcc'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Send as BCC (hide recipients)'),
      '#default_value' => 0,
    );
    $form['copy_yourself'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Send a copy to yourself'),
      '#default_value' => 0
    );
    $form['submit'] = array(
      '#type'  => 'submit',
      '#value' => t('Send e-mail'),
    );
  }
  else {
    $form['error'] = array(
      '#value' => '<p><b>'. t('You must create at least one category before using this form.') .'</b>',
    );
  }

  return $form;
}



/**
 * Validate the site-wide contact page form submission.
 */
function email_league_form_validate($form, &$form_state) {
  // required fields are already checked.
  //if (!$form_state['values']['league_id']) {
  //  form_set_error('league_id', t('You must select at least one league.'));
  //}
}


function email_league_form_submit($form, &$form_state) {
  global $user;

  $bcc = $form_state['values']['bcc'];
  $copy_yourself = $form_state['values']['copy_yourself'];

  $return_address = variable_get('pickem_commisioner_email', '');
  if ( $return_address == '' ) {
    $return_address = $user->mail;
  }

  $headers['From'] = $headers['Reply-To'] = $headers['Sender'] = $headers['Return-Path'] = $headers['Errors-To'] = $return_address;

  $headers['Content-Type'] = 'text/plain; charset=UTF-8; format=flowed';
  //$headers['Content-Type'] = 'text/html; charset=UTF-8;';

  // Add in the actual message
  $message[] = wordwrap($form_state['values']['message']);

  // Format the subject line:
  $subject = 'Sports Pickem: '. $form_state['values']['subject'];

  // Prepare the body:
  $body = implode("\n\n", $message);

  // Load the recipients
  $recipients = array();

  // Add the logged-in user if they want a copy.
  if ($copy_yourself == 1) {
    array_push($recipients, $user);
  }

  foreach ($form_state['values']['league_id'] as $league_id) {
    $league = league_load($league_id);
    $recipients = array_merge($recipients, get_users($league));
  }
  if (count($recipients) == 0) {
    drupal_set_message(t('There are no users in this league. Mail not sent.'));
    return '';
  }
  $recipients_str = '';
  foreach ($recipients as $r) {
    $recipients_str .= $r->name ." <". $r->mail .">,";
  }

  // set bcc
  if ($bcc == 1) { // hidden recipients
    $headers['Bcc'] = $recipients_str;
    $to = $from;
  }
  else {
    $to = $recipients_str;
  }

  $params = array();
  $params['headers'] = $headers;
  $params['subject'] = $subject;
  $params['body'] = $body;

  $message = drupal_mail('pickem', 'notice', $to, language_default(), $params);
  if ($message['result'] == 1) {
    $t_string = t('Email sent.');
    drupal_set_message($t_string);
    watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);
  }
  else {
    $t_string = t('Error sending email.');
    drupal_set_message($t_string);
    watchdog('pickem', $t_string, NULL, WATCHDOG_ERROR);
  }

}


/*
  Path:     admin/pickem/scores
*/
function page_admin_scores($league, $week) {

  // still old school for now, look at fapi
  if (count($_POST) > 0) {
    $type = "";
    foreach ($_POST as $gid => $score) {
      // example of $gid:  (gid_1_vid_123)
      // [0]=gid label [1]=gid, [2]=team label, [3]=team_id
      $parts = explode('_', $gid);

      $gid = $parts[1];
      $team_type = $parts[2];
      $team_id = $parts[3];

      $query = '';
      if ($team_type == 'vid') {
        // update visitor score
        $query = "update {pickem_games} set v_score=%d where gid=%d";
      }
      elseif ($team_type == 'hid') {
        $query = "update {pickem_games} set h_score=%d where gid=%d";
      }
      // can't clear score right now
      if ($score <> "" && $query <> "") {
        $result = db_query($query, $score, $gid);
      }
    }

    // clear pickem_ caches, since you are updating scores here.
    cache_clear_all('pickem_', 'cache', TRUE);

    drupal_set_message("Scores Saved");
  }

  $games = get_games(array('week_id' => $week->wid));
  $teams = get_teams(array('gameset_id' => $league->gsid));
  $output = '<div class="pickem"><form action="" method="post"><table class="admin-scores">';
  $output .= '<tr><th colspan="2">Visitor</th><th colspan="2">Home</th><th>Game Time(EST)</th></tr>';
  foreach ($games as $gid => $game) {
    $output .= '<tr>';
    $output .= '<td class="team">'. $teams[$game->v_id]->short_name .'</td><td class="score"><input name="gid_'. $gid .'_vid_'. $game->v_id .'" type="text" size="3" value="'. $game->v_score .'" /></td>';
    $output .= '<td class="team">'. $teams[$game->h_id]->short_name .'</td><td class="score"><input name="gid_'. $gid .'_hid_'. $game->h_id .'" type="text" size="3" value="'. $game->h_score .'" /></td>';
    $output .= '<td class="time">'. $game->gametime .'</td></tr>';
  }
  $output .= '</table>';
  $output .= '<input type="submit" value="Save Scores"/>';
  $output .= '</form></div>';


  print theme('page', $output);
}

function league_forums() {
  $options = array();
  $options[0] = 'No Forum Selected';
  if (module_exists("forum")) {
  $forums = forum_get_forums();
    if ($forums) {
      foreach ($forums as $tid => $forum) {
         $options[$tid] = $forum->name;
      }
    }
  }
  return $options;
}

function gameset_options() {
  $options = array();
  $gamesets = get_gamesets();
  if ($gamesets) {
    foreach ($gamesets as $gsid => $gameset) {
       $options[$gsid] = $gameset->gs_name;
    }
  }
  return $options;
}




/*
  Import Functions
*/
function page_admin_data_import() {
  drupal_set_title('Pickem Import Data');
  return drupal_get_form('admin_import_form');
}
function admin_import_form($form_state=NULL) {

  $dirpath = drupal_get_path('module', 'pickem') ."/data/";
  $dir = opendir($dirpath);
  $list = array();
  while ($filename = readdir($dir)) {
    if ($filename != '.' && $filename != '..') {
      $fullpath = drupal_get_path('module', 'pickem') ."/data/" . $filename;
      $f = fopen($fullpath, "r");
      $string = fread($f, filesize($fullpath));
      fclose($f);
      $gs = unserialize($string);
      $teams = 'no';
      $weeks = 'no';
      $games = 'no';
      if ( count($gs['teams']) > 0 ) {
        $teams = 'yes';
      }
      if ( count($gs['weeks']) > 0 ) {
        $weeks = 'yes';
      }
      if ( count($gs['games']) > 0 ) {
        $games = 'yes';
      }
      $list[] = array('filename' => $filename, 'teams' => $teams, 'weeks' => $weeks, 'games' => $games);
    }
  }
  closedir($dir);

  $gamesets = array();
  $options = array();
  foreach ( $list as $file ) {
    $tmp = explode(".", $file['filename']);
    array_pop($tmp);
    $setname = implode(".", $tmp);
    $gamesets[$setname] = $file;
    $options[$setname] = $setname ." (teams=". $file['teams'] .", weeks=". $file['weeks'] .", games=". $file['games'] .")";
  }

  $form['pickem_data_import_gameset_name'] = array(
    '#type' => 'radios',
    '#title' => t('Choose game set to load'),
    '#options' => $options,
    '#description' => t('This will create a new game set including the data from the import file.'),
  );

  $form['pickem_load_data_button'] = array(
    '#type' => 'submit',
    '#value' => t('Load data'),
  );

  $form['#gamesets'] = $gamesets;

  return $form;
}

function admin_import_form_submit($form, &$form_state) {
  $return = TRUE;
  $gameset_name = $form_state['values']['pickem_data_import_gameset_name'];
  $gameset = $form['#gamesets'][$gameset_name];
  unserialize_gameset($gameset, $gameset_name);

  if ( $return == TRUE ) {
    drupal_set_message("Game Set imported", "status");
  }
  else {
    drupal_set_message("Game Set import FAILED", "error");
  }

  drupal_goto('admin/pickem/gamesets');
}


function unserialize_gameset($gameset, $gameset_name) {
  $return = TRUE;

  $fullpath = drupal_get_path('module', 'pickem') ."/data/" . $gameset['filename'];
  $file = fopen($fullpath, "r");
  $string = fread($file, filesize($fullpath));
  fclose($file);
  $gs = unserialize($string);
  // $gs['teams'] = get_teams();
  // $gs['weeks'] = get_weeks();
  // $gs['games'] = get_games();

  // GAMESET
  $gsid = 0;
  $result = db_query("insert into {pickem_gamesets} (gs_name) values('%s')", $gameset_name);
  if ( $result != FALSE ) {
    $gsid = db_last_insert_id('pickem_gamesets', 'gsid');
    // touch up the game set name so that it is unique and easier to pick out, tack on the id
    $result = db_query("UPDATE {pickem_gamesets} set gs_name='%s' where gsid=%d", $gameset_name .'-'. $gsid, $gsid);
  }

  // TEAMS
  foreach ( $gs['teams'] as $tid => $team ) {
    $query = "insert into {pickem_teams} (gsid, tm_city, tm_abbrev, tm_name, tm_nick, conference, division, logo_small) values(%d, '%s','%s','%s','%s','%s','%s','%s')";
    $result = db_query($query, $gsid, $team->tm_city, $team->tm_abbrev, $team->tm_name, $team->tm_nick, $team->conference, $team->division, $team->logo_small);
    if ( $result == FALSE ) {
      drupal_set_message("Insert teams failed", "error");
      $return = FALSE;
    }
    // save the new tid into the datastructure for the games inserts
    $new_tid = db_last_insert_id('pickem_teams', 'tid');
    $gs['teams'][$tid]->tid = $new_tid;
  }

  // WEEKS
  foreach ( $gs['weeks'] as $wid => $week ) {
    $query = "insert into {pickem_weeks} (wk_number, gsid, wk_name, wk_abbrev, wk_firstgame, wk_points, playoffs) values(%d, %d, '%s', '%s', '%s', %d, %d)";
    $result = db_query($query, $week->wk_number, $gsid, $week->wk_name, $week->wk_abbrev, date($week->wk_firstgame), $week->wk_points, $week->playoffs);
    if ( $result == FALSE ) {
      drupal_set_message("Insert weeks failed", "error");
      $return = FALSE;
    }
    // save the new wid into the datastructure for the games inserts
    $new_wid = db_last_insert_id('pickem_weeks', 'wid');
    $gs['weeks'][$wid]->wid = $new_wid;
  }

  // GAMES, use the new tid and new wid
  foreach ( $gs['games'] as $gid => $game ) {

    // drupal bug #245335, can't insert nulls with db_query replacement
    $game->v_score = is_null($game->v_score) ? 'NULL' : (int) $game->v_score;
    $game->h_score = is_null($game->h_score) ? 'NULL' : (int) $game->h_score;

    $query = "insert into {pickem_games}(wid, gsid, gametime, h_id, h_spread, h_score, v_id, v_spread, v_score) values(%d, %d, '%s', %d, %d, %s, %d, %d, %s)";
    $result = db_query($query, $gs['weeks'][$game->wid]->wid, $gsid, date($game->gametime), $gs['teams'][$game->h_id]->tid, $game->h_spread, $game->h_score, $gs['teams'][$game->v_id]->tid, $game->v_spread, $game->v_score);
    if ( $result == FALSE ) {
      drupal_set_message("Insert games failed", "error");
      $return = FALSE;
    }
  }

  return $return;
}


/**
 * EXPORT FUNCTIONS, will need to sanitize those filenames
 */
function export_gameset($gameset) {

  $gs = array();
  $gs['gameset'] = $gameset;
  $gs['teams'] = get_teams(array('gameset_id' => $gameset->gsid));
  $gs['weeks'] = get_weeks(array('gameset_id' => $gameset->gsid));
  $gs['games'] = get_games(array('gameset_id' => $gameset->gsid));


  $gameset_name = $gameset->gs_name;
  $gameset_name = str_replace(" ", "_", $gameset_name);
  $filename = $gameset_name .'.gameset';
  $fullpath = drupal_get_path('module', 'pickem') .'/data/'. $filename;
  $i=65;  // uppercase A
  // try not to overwrite existing export files.
  while (file_exists($fullpath)) {
    $gameset_name = $gameset->gs_name;
    $gameset_name = str_replace(" ", "_", $gameset_name);
    $filename = $gameset_name .'-'. chr($i) .'.gameset';
    $fullpath = drupal_get_path('module', 'pickem') .'/data/'. $filename;
    if ( $i >= 90 ) {
      break;
    }
    $i++;
  }

  $file = fopen($fullpath, "w");

  $string = serialize($gs);
  fwrite($file, $string);
  fclose($file);

  $t_string = t('Exported game set %gameset-name to %export-dir', array('%gameset-name' => $gameset_name, '%export-dir' => $fullpath));
  drupal_set_message($t_string);
  watchdog('pickem', $t_string, NULL, WATCHDOG_NOTICE);
  drupal_goto('admin/pickem/gamesets');
}
