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
 
  menu_cache_clear_all();
  cache_clear_all();
  menu_rebuild();
   
  $content = 'Pickem Overview';
  return $content;
}

function pickem_admin_settings(&$form_state) {

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
    '#title' => t('Default email reminders'),
    '#return_value' => 1,
    '#default_value' => variable_get('pickem_default_email_reminders', 1),
    '#description' => t("Default email reminder setting for new users.  Each user can override this."),
  );

  $form['pickem_partial_league'] = array(
    '#type' => 'checkbox',
    '#title' => t('Partial league support'),
    '#return_value' => 1,
    '#default_value' => variable_get('pickem_partial_league', 0),
    '#description' => t("If you don't pick every game for every team, use this setting.)"),
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

  $form['pickem_tz'] = array(
    '#type' => 'fieldset',
    '#title' => t('Pickem Time Zone'),
    '#collapsible' => TRUE,
  );
  
  $form['pickem_tz']['pickem_time_zone'] = array(
    '#type' => 'select',
    '#default_value' => variable_get('pickem_time_zone', 'TZ=US/Eastern'),
    '#options' => array('TZ=US/Eastern'),
    '#title' => t('Time zone for pickem weeks and games'),
    '#description' => 'Current Server Time='. date("r"),
  );


  return system_settings_form($form);
}

function page_admin_leagues_users($league_id = "") {
  
  if ( is_numeric($_POST['league_select']) ) {
    drupal_goto("admin/content/pickem/league_users/" . $_POST['league_select']);
  }
  
  drupal_set_title('Choose a League to administer');
  return drupal_get_form('league_select_form');
}


function page_admin_league_users($league) {
  /* 
    Post Actions
  */
  if ( $_POST['user_select'] && $_POST['op'] == t('Add') ) {
  // user_select
    $u = user_load(array('uid' => $_POST['user_select']));
    db_query("insert into {pickem_users} (lid,uid) values (%d,%d)", $league->lid, $u->uid);
    drupal_set_message($u->name .' added to '. $league->name);
    drupal_goto("admin/content/pickem/league_users/". $league->lid);
  }

  if ( arg(5) == "delete_user" && is_numeric(arg(6)) ) {
    $u = user_load(array('uid' => arg(6)));
    db_query("delete from {pickem_users} where lid=%d and uid=%d", $league->lid, $u->uid);
    drupal_set_message($u->name .' removed from '. $league->name);
    drupal_goto("admin/content/pickem/league_users/". $league->lid);
  }


  /*
    Page Display
  */
  $header = array(t('Name'), t('Operations'));
  $rows = array();
  
  $query = "SELECT u.uid,name from {users} u inner join {pickem_users} pu on pu.uid=u.uid where u.uid<>0 and lid=%d order by u.uid";
  $result = db_query($query, $league->lid);
  while ($row = db_fetch_object($result)) {
    $rows[] = array(l($row->name, 'user/'. $row->uid), l(t('delete user'), 'admin/content/pickem/league_users/'. $league->lid .'/delete_user/'. $row->uid));
  }
  $output = theme('table', $header, $rows);
  $output .= drupal_get_form('user_select_form', $league->lid, 'Add User', 1);
 
  drupal_set_title('Administer League - '. $league->name);
  return $output;
}




/*
  Admin Games
*/
function page_admin_games() {
  drupal_set_title('Pickem Games Admin');
  return drupal_get_form('admin_games_form');
}
function admin_games_form($form_state=NULL) {
  $form = array(
    '#theme' => 'admin_games_form',
    '#tree' => TRUE,
  );  

  $games = get_games();
  $teams = get_teams();
  $team_list = array();
  $team_list[0] = "";
  foreach ($teams as $t_id => $team_data) {
    $team_list[$t_id] = ($team_data->tm_abbrev <> '')?$team_data->tm_abbrev:$team_data->tm_name;
  }

  $weeks = get_weeks(0, 'all');
  $week_list = array();
  $week_list[0] = "";
  foreach ($weeks as $wid => $week) {
    $week_list[$wid] = $week['wk_name'];
  }

  foreach ($games as $gid => $game_data) {
    $form['games'][] = array(
      'gid' => array(
        '#type' => 'hidden',
        '#value' => $gid
      ),
      'wid' => array(
        '#type' => 'select',
        '#default_value' => $game_data->wid,
        '#options' => $week_list,
      ),
      'game_time' => array(
        '#type' => 'textfield',
        '#size' => 30,
        '#maxlength' => 45,
        '#default_value' => $game_data->gametime,
      ),
      'home_team' => array(
        '#type' => 'select',
        '#default_value' => $game_data->h_id,
        '#options' => $team_list,
      ),
      'h_spread' => array(
        '#type' => 'textfield',
        '#size' => 4,
        '#maxlength' => 10,
        '#default_value' => $game_data->h_spread,
      ),
      'visitor_team' => array(
        '#type' => 'select',
        '#default_value' => $game_data->v_id,
        '#options' => $team_list,
      ),
      'v_spread' => array(
        '#type' => 'textfield',
        '#size' => 4,
        '#maxlength' => 10,
        '#default_value' => $game_data->v_spread,
      ),
    );
  }

  // add new game
  $form['new_game']['game'] = array(
    'gid' => array(
      '#type' => 'hidden',
      '#value' => 0
    ),
    'wid' => array(
      '#type' => 'select',
      '#default_value' => 0,
      '#options' => $week_list,
    ),
    'game_time' => array(
      '#type' => 'textfield',
      '#size' => 30,
      '#maxlength' => 45,
      '#default_value' => "",
    ),
    'home_team' => array(
      '#type' => 'select',
      '#default_value' => "",
      '#options' => $team_list,
    ),
    'h_spread' => array(
      '#type' => 'textfield',
      '#size' => 4,
      '#maxlength' => 10,
      '#default_value' => 0,
    ),
    'visitor_team' => array(
      '#type' => 'select',
      '#default_value' => "",
      '#options' => $team_list,
    ),
    'v_spread' => array(
      '#type' => 'textfield',
      '#size' => 4,
      '#maxlength' => 10,
      '#default_value' => 0,
      ),
  );

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save Games'),
   );
  return $form; 
}


function theme_admin_games_form(&$form) {
  //drupal_set_message('theme_admin_games_form');
  
  $week = $_GET['page'] ? $_GET['page'] : 0;

  $rows = array();
  foreach (element_children($form['games']) as $key) {
    if ($form['games'][$key]['wid']['#value'] == $week + 1) {
      $rows[] = array(
        drupal_render($form['games'][$key]['wid']), 
        drupal_render($form['games'][$key]['game_time']), 
        drupal_render($form['games'][$key]['visitor_team']), 
        drupal_render($form['games'][$key]['v_spread']), 
        drupal_render($form['games'][$key]['home_team']),
        drupal_render($form['games'][$key]['h_spread']), 
      );
    }
  }

  $new_row = array();
  foreach (element_children($form['new_game']) as $key) {
    $form['new_game'][$key]['wid']['#value'] = $week + 1;  //set the new_game form element to be in the week currently being viewed.
    $new_row[] = array(
      drupal_render($form['new_game'][$key]['wid']), 
      drupal_render($form['new_game'][$key]['game_time']), 
      drupal_render($form['new_game'][$key]['visitor_team']), 
      drupal_render($form['new_game'][$key]['v_spread']), 
      drupal_render($form['new_game'][$key]['home_team']),
      drupal_render($form['new_game'][$key]['h_spread']), 
    );
  }

  $header = array(t('Week'), t('Time'), t('Visitor'), t('VSpread'), t('Home'), t('HSpread'));
  $output = '';
  $output .= '<div><fieldset class="collapsible"><legend><a href="#">Games</a></legend>';
  $output .= theme('table', $header, $rows);
  $output .= "</fieldset></div>";
  
  $output .= '<div><fieldset class="collapsible"><legend><a href="#">New game</a></legend>';
  $output .= theme('table', $header, $new_row);
  $output .= "</fieldset></div>";

  $output .= drupal_render($form['save']);
  $output .= drupal_render($form['form_id']);
  $output .= drupal_render($form['form_token']);

  $GLOBALS['pager_page_array'][] = $week;
  $weeks = get_weeks(0, 'all');
  $GLOBALS['pager_total'][] = count($weeks);

  #if ($total_entries >= $page_increment) {
  $output .= theme('pager', NULL, $page_increment);
  #}

  drupal_add_js('misc/collapse.js');

  return $output;
}
function admin_games_form_submit($form, &$form_state) {
  //drupal_set_message('admin_games_form_submit');

  cache_clear_all('pickem_grid_scores', 'cache', TRUE); 

  if ( isset($form_state['values']['games']) ) {
    foreach ($form_state['values']['games'] as $key => $value) {
      // this will filter by games on the current page only.
      if ($value['gid'] <> 0 && $value['game_time'] != '' && $value['home_team'] <> 0 && $value['visitor_team'] <> 0) {
        $query = "update {pickem_games} set wid=%d, gametime='%s', h_id=%d, h_spread=%d, v_id=%d, v_spread=%d where gid=%d";
        $result = db_query($query, $value['wid'], date($value['game_time']), $value['home_team'], $value['h_spread'], $value['visitor_team'], $value['v_spread'], $value['gid']);
      }
    }
  }

  foreach ($form_state['values']['new_game'] as $key => $value) {
    if ($value['gid'] == 0 && $value['game_time'] != '' && $value['home_team'] <> 0 && $value['visitor_team'] <> 0) {
      $query = "insert into {pickem_games}(wid, gametime, h_id, h_spread, v_id, v_spread) values(%d, '%s', %d, %d, %d, %d)";
      $result = db_query($query, $value['wid'], date($value['game_time']), $value['home_team'], $value['h_spread'], $value['visitor_team'], $value['v_spread']);
      drupal_set_message("Games Saved");
      drupal_goto('admin/content/pickem/games', 'page='. ($value['wid'] - 1));
    }
  }
  drupal_set_message("Games Saved");
}







/*
  Admin Weeks
*/
function page_admin_weeks() {
  drupal_set_title('Pickem Weeks Admin');
  return drupal_get_form('admin_weeks_form');
}
function admin_weeks_form($form_state=NULL) {
  //drupal_set_message('admin_weeks_form');
  $form = array(
    '#theme' => 'admin_weeks_form',
    '#tree' => TRUE,
  );  
  
  $query = "SELECT wid, wk_name, wk_abbrev, wk_firstgame, wk_points, playoffs from {pickem_weeks} ORDER BY wid";
  $result = db_query($query);
  while ($row = db_fetch_object($result)) {
    $form['weeks'][] = array(
      'wid' => array(
        '#type' => 'hidden',
        '#value' => $row->wid
      ),
      'num' => array(
        '#type' => 'textfield',
        '#size' => 3,
        '#maxlength' => 3,
        '#default_value' => $row->wid
      ),
      'name' => array(
        '#type' => 'textfield',
        '#size' => 30,
        '#maxlength' => 45,
        '#default_value' => $row->wk_name,
      ),
      'abbrev' => array(
        '#type' => 'textfield',
        '#size' => 10,
        '#maxlength' => 15,
        '#default_value' => $row->wk_abbrev,
      ),
      'firstgame' => array(
        '#type' => 'textfield',
        '#size' => 20,
        '#maxlength' => 35,
        '#default_value' => $row->wk_firstgame,
      ),
      'points' => array(
        '#type' => 'textfield',
        '#size' => 5,
        '#maxlength' => 5,
        '#default_value' => $row->wk_points,
      ),
      'playoffs' => array(
        '#type' => 'radios',
        '#default_value' => $row->playoffs,
        '#options' => array( 0 => t('Regular Season'), 1 => t('Playoffs') )
      )
      
    );
  }
  // add new week
  $form['new_row'] = array(
    'wid' => array(
      '#type' => 'textfield',
      '#size' => 3,
      '#maxlength' => 3,
      '#default_value' => ""
    ),
    'name' => array(
      '#type' => 'textfield',
      '#size' => 30,
      '#maxlength' => 45,
      '#default_value' => "",
    ),
    'abbrev' => array(
      '#type' => 'textfield',
      '#size' => 10,
      '#maxlength' => 15,
      '#default_value' => "",
    ),
    'firstgame' => array(
      '#type' => 'textfield',
      '#size' => 20,
      '#maxlength' => 35,
      '#default_value' => "",
    ),
    'points' => array(
      '#type' => 'textfield',
      '#size' => 5,
      '#maxlength' => 5,
      '#default_value' => "",
    ),
    'playoffs' => array(
      '#type' => 'radios',
      '#default_value' => 0,
      '#options' => array( 0 => t('Regular Season'), 1 => t('Playoffs') )
    )
  );

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save Weeks')
  );
  return $form;  
}
function theme_admin_weeks_form(&$form) {
  //drupal_set_message('theme_admin_weeks_form');
  
  drupal_add_js('misc/collapse.js');
  $header = array( t('Num'), t('Name'), t('Abbrev'), t('FirstGame'), t('Points'));
  $rows = array();
  foreach (element_children($form['weeks']) as $key) {
    $rows[] = array(
      drupal_render($form['weeks'][$key]['wid']) .
      drupal_render($form['weeks'][$key]['num']), 
      drupal_render($form['weeks'][$key]['name']), 
      drupal_render($form['weeks'][$key]['abbrev']), 
      drupal_render($form['weeks'][$key]['firstgame']), 
      drupal_render($form['weeks'][$key]['points']),
      drupal_render($form['weeks'][$key]['playoffs']),
    );
  }
  $output = '';
  $output .= '<div><fieldset class="collapsible"><legend><a href="#">Weeks</a></legend>';
  $output .= theme('table', $header, $rows);
  $output .= "</fieldset></div>";

  $new_row = array();
  $new_row[] = array(
    drupal_render($form['new_row']['wid']) .
    drupal_render($form['new_row']['num']), 
    drupal_render($form['new_row']['name']), 
    drupal_render($form['new_row']['abbrev']), 
    drupal_render($form['new_row']['firstgame']), 
    drupal_render($form['new_row']['points']),
    drupal_render($form['new_row']['playoffs']),
  );

  $output .= '<div><fieldset class="collapsible"><legend><a href="#">Add New Week</a></legend>';
  $output .= theme('table', $header, $new_row);
  $output .= "</fieldset></div>";

  $output .= drupal_render($form['save']);
  $output .= drupal_render($form['form_id']);
  $output .= drupal_render($form['form_token']);

  return $output;
}

function admin_weeks_form_submit($form, &$form_state) {
  //drupal_set_message('admin_weeks_form_submit');
  
  if ( isset($form_state['values']['weeks']) ) {
    foreach ($form_state['values']['weeks'] as $key => $value) {
        $query = "update {pickem_weeks} set wid=%d, wk_name='%s', wk_abbrev='%s', wk_firstgame='%s', wk_points=%d, playoffs=%d where wid=%d";
        $result = db_query($query, $value['num'], $value['name'], $value['abbrev'], date($value['firstgame']), $value['points'], $value['playoffs'], $value['wid']);
    }
  }

  if ( $form_state['values']['new_row']['wid'] <> '' ) {
    $value = $form_state['values']['new_row'];
    $query = "insert into {pickem_weeks}(wid, wk_name, wk_abbrev, wk_firstgame, wk_points, playoffs) values(%d, '%s', '%s', '%s', %d, %d)";
    $result = db_query($query, $value['wid'], $value['name'], $value['abbrev'], date($value['firstgame']), $value['points'], $value['playoffs']);
  }

  cache_clear_all('pickem_grid_scores', 'cache', TRUE); 
  drupal_set_message("Weeks Saved");
}



/*
  Admin Teams
*/
function admin_teams_form($form_state=NULL) {
  //drupal_set_message("admin_teams_form");

  $form = array(
    '#theme' => 'admin_teams_form',
    '#tree' => TRUE,
  );  

  $query = "SELECT tid, tm_city, tm_abbrev, tm_nick, tm_name, conference, division from {pickem_teams} order by tm_city, tm_name, tm_abbrev";
  $result = db_query($query);
  while ($row = db_fetch_object($result)) {
    $form['teams'][$row->tid] = array(
      'tm_city' => array(
        '#value' => $row->tm_city
      ),
      'tm_name' => array(
        '#value' => $row->tm_name
      ),
      'tm_abbrev' => array(
        '#value' => $row->tm_abbrev
      ),
      'edit_link' => array(
        '#value' => l('edit', 'admin/content/pickem/teams/edit/' . $row->tid)
      ),
      'delete_link' => array(
        '#value' => l('delete', 'admin/content/pickem/teams/delete/' . $row->tid)
      ),
    );
  }
  
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save Teams')
  );

  return $form;
}
function theme_admin_teams_form(&$form) {
  $header = array(
    t('Team'), 
    array('data' => t('Operations'), 'colspan' => '3')
  );

  $rows = array();
  foreach (element_children($form['teams']) as $key) {
    $rows[] = array(
      drupal_render($form['teams'][$key]['tm_city']) . ' ' . drupal_render($form['teams'][$key]['tm_name']) . ' (' . drupal_render($form['teams'][$key]['tm_abbrev']) . ')', 
      drupal_render($form['teams'][$key]['edit_link']),
      drupal_render($form['teams'][$key]['delete_link']),
    );
  }
 
  $output = theme('table', $header, $rows);
  
  return $output;
}



/**
 * Menu callback; Build the pickem single team editing form.
 */
function pickem_team_edit(&$form_state, $type, $team) {

  if ( is_null($team) ) {
  	// this should match the table field names
    $team = new stdClass();
  	$team->tid = 0;
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
	
  $form['team']['tm_city'] = array(
      '#title'  => t('City'),
      '#type' => 'textfield',
      '#size' => 20,
      '#maxlength' => 50,
      '#default_value' => $team->tm_city
  );
  $form['team']['tm_abbrev'] = array(
    '#title'  => t('Abbreviation'),
    '#type' => 'textfield',
    '#size' => 5,
    '#maxlength' => 5,
    '#default_value' => $team->tm_abbrev
  );
  $form['team']['tm_nick'] = array(
    '#title'  => t('Nickname'),
    '#type' => 'textfield',
    '#size' => 35,
    '#maxlength' => 50,
    '#default_value' => $team->tm_nick
  );
  $form['team']['tm_name'] = array(
    '#title'  => t('Name'),
    '#type' => 'textfield',
    '#size' => 15,
    '#maxlength' => 50,
    '#default_value' => $team->tm_name
  );
  $form['team']['conference'] = array(
    '#title'  => t('Conference'),
    '#type' => 'textfield',
    '#size' => 10,
    '#maxlength' => 50,
    '#default_value' => $team->conference
  ); 
  $form['team']['division'] = array(
    '#title'  => t('Division'),
    '#type' => 'textfield',
    '#size' => 10,
    '#maxlength' => 50,
    '#default_value' => $team->division
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
    $query = "insert into {pickem_teams} (tm_city, tm_abbrev, tm_name, tm_nick, conference, division) values('%s','%s','%s','%s','%s','%s')";
    $result = db_query($query, $team['tm_city'], $team['tm_abbrev'], $team['tm_name'], $team['tm_nick'], $team['conference'], $team['division']);
		drupal_set_message('Team added');
	}
	else {
	  $query = "update {pickem_teams} set tm_city='%s', tm_abbrev='%s', tm_name='%s', tm_nick='%s', conference='%s', division='%s' where tid=%d";
	  $result = db_query($query, $team['tm_city'], $team['tm_abbrev'], $team['tm_name'], $team['tm_nick'], $team['conference'], $team['division'], $team['tid']);
	  drupal_set_message('Team updated');
	}

  $form_state['redirect'] = 'admin/content/pickem/teams';
}
/**
 * Build a confirm form for deletion of a team.
 */
function pickem_team_delete_confirm(&$form_state, $team) {
  $form['#team'] = $team;
  $caption = '';
  $caption .= '<p>'. t('This action cannot be undone.') .'</p>';
  return confirm_form($form, t('Are you sure you want to delete the team %name?', array('%name' => $team->tm_city . ' ' . $team->tm_name)), 'admin/content/pickem/teams', $caption, t('Delete'));
}
/**
 * Delete the team.
 */
function pickem_team_delete_confirm_submit($form, &$form_state) {
  $team = $form['#team'];
  db_query("DELETE FROM {pickem_teams} WHERE tid = %d", $team->tid);
  $t_args = array('%name' => $team->tm_city . ' ' . $team->tm_name);
  drupal_set_message(t('The pickem team %name has been deleted.', $t_args));
  watchdog('pickem', 'Deleted team %name.', $t_args, WATCHDOG_NOTICE);
  $form_state['redirect'] = 'admin/content/pickem/teams';
}











/**************
  Admin Leagues
***************/
function pickem_leagues_list(&$form_state) {
  //drupal_set_message("admin_leagues_form");

  $form = array(
    '#theme' => 'pickem_leagues_list',
    '#tree' => TRUE,
  );  

  $query = "SELECT * from {pickem_leagues} order by lid";
  $result = db_query($query);
  
  while ($row = db_fetch_object($result)) {
    $form['leagues'][$row->lid] = array(
      'name' => array(
        '#value' => $row->name
      ),
      'edit_link' => array(
        '#value' => l('edit', 'admin/content/pickem/leagues/edit/' . $row->lid)
      ),
      'delete_link' => array(
        '#value' => l('delete', 'admin/content/pickem/leagues/delete/' . $row->lid)
      ),
    );
  }
  
  /*
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save Leagues')
  );
  */

  return $form;
}
function theme_pickem_leagues_list(&$form) {

  $header = array(
    t('League'), 
    array('data' => t('Operations'), 'colspan' => '3')
  );

  $rows = array();
  foreach (element_children($form['leagues']) as $key) {
    $rows[] = array(
      drupal_render($form['leagues'][$key]['name']), 
      drupal_render($form['leagues'][$key]['edit_link']),
      drupal_render($form['leagues'][$key]['delete_link'])
    );
  }

  $output .= theme('table', $header, $rows);

  $output .= drupal_render($form['save']);
  //$output .= drupal_render($form['form_id']);
  //$output .= drupal_render($form['form_token']);
  
  return $output;
}
function admin_leagues_form_submit($form, &$form_state) {
  //drupal_set_message("admin_league_form_submit");
  
  if ( isset($form_state['values']['leagues']) ) {
    foreach ($form_state['values']['leagues'] as $key => $value) {
      $query = "update {pickem_leagues} set name='%s', rules_node_id=%d, forum_id=%d, scoring_type=%d where lid=%d";
      $result = db_query($query, $value['name'], $value['rules_node_id'], $value['forum_id'], $value['scoring_type'], $value['lid']);
    }
  }
  
  if ( $form_state['values']['add_league']['name'] <> '' ) {
    $value = $form_state['values']['add_league'];
    $query = "insert into {pickem_leagues} (name, rules_node_id, forum_id, scoring_type) values('%s', %d, %d, %d)";
    $result = db_query($query, $value['name'], $value['rules_node_id'], $value['forum_id'], $value['scoring_type']);
  }
  
  drupal_set_message("Leagues Saved");
}

/**
 * LEAGUES
 * Menu callback; Build the pickem league editing form.
 */
function pickem_leagues_edit(&$form_state, $type, $league) {

  if ( is_null($league) ) {
  	// this should match the table field names
    $league = new stdClass();
  	$league->lid = 0;
  	$league->name = '';
  	$league->forum_id = 0;
  	$league->rules_node_id = '';
  	$league->scoring_type = 0;
  }
  $league_forums_options = league_forums();
  
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
    '#options' => array( 0 => t('Win-Loss'), 1 => t('Spread') ),
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}
/**
 * Validate form values for a league being added or edited.
 */
function pickem_leagues_edit_validate($form, &$form_state) {
  $league = &$form_state['values'];
  // not used since it is required, keep this for a while.
  if (empty($league['name'])) {
  	form_set_error('name', t('You must at least choose a name.'));
  }
}
/**
 * Submit form values for a league being added or edited.
 */
function pickem_leagues_edit_submit($form, &$form_state) {
  $league = $form_state['values'];

	if ( $league['lid'] == 0 ) {
    $query = "insert into {pickem_leagues} (name, rules_node_id, forum_id, scoring_type) values('%s', %d, %d, %d)";
    $result = db_query($query, $league['name'], $league['rules_node_id'], $league['forum_id'], $league['scoring_type']);
		drupal_set_message('Team added');
	}
	else {
    $query = "update {pickem_leagues} set name='%s', rules_node_id=%d, forum_id=%d, scoring_type=%d where lid=%d";
    $result = db_query($query, $league['name'], $league['rules_node_id'], $league['forum_id'], $league['scoring_type'], $league['lid']);
	  drupal_set_message('Team updated');
	}

  $form_state['redirect'] = 'admin/content/pickem/leagues';
}
/**
 * Build a confirm form for deletion of a league.
 */
function pickem_leagues_delete_confirm(&$form_state, $league) {
  $form['#league'] = $league;
  $caption = '';
  $caption .= '<p>'. t('This action cannot be undone.') .'</p>';
  return confirm_form($form, t('Are you sure you want to delete the team %name?', array('%name' => $league->name)), 'admin/content/pickem/leagues', $caption, t('Delete'));
}
/**
 * Delete the league.
 */
function pickem_leagues_delete_confirm_submit($form, &$form_state) {
  $league = $form['#league'];
  db_query("DELETE FROM {pickem_leagues} WHERE lid = %d", $league->lid);
  $t_args = array('%name' => $league->name);
  drupal_set_message(t('The pickem league %name has been deleted.', $t_args));
  watchdog('pickem', 'Deleted league %name.', $t_args, WATCHDOG_NOTICE);
  $form_state['redirect'] = 'admin/content/pickem/leagues';
}






/*
  implementation of hook_mail
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



/*
  Good General Use Forms
*/
function league_select_form($form_state) {
  $leagues = get_leagues();
  $options = array();
  foreach ($leagues as $l) {
    $options[$l->lid] = $l->name; 
  }
  
  $form['league_select'] = array( '#type' => 'select', '#title' => t('Select League'), '#default_value' => 1, '#options' => $options);
  $form['op'] = array(
    '#type' => 'button',
    '#value' => t('Choose')
  );
  return $form;
}
function user_select_form($form_state=NULL, $league_id=0, $title='Select User To Modify', $not_in_league=0) {
  
  //drupal_set_message('test user_select_form');
  
  $users = get_users($league_id, $not_in_league);
  $user_list = array();
  foreach ($users as $u_id => $user_data) {
    $user_list[$u_id] = $user_data["name"];
  }
  
  $form = array();
  $form['user_select'] = array(
    '#type' => 'select',
    '#title' => t($title), 
    '#default_value' => 1, 
    '#options' => $user_list,
  );
  $op = explode(' ', $title);
  $form['op'] = array(
    '#type' => 'button',
    '#value' => t($op[0])
  );
  return $form;
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

  $bcc = $form_state['values']['bcc'];

  $headers['From'] = $headers['Reply-To'] = $headers['Sender'] = $headers['Return-Path'] = $headers['Errors-To'] = variable_get('pickem_commisioner_email', '');

  $headers['Content-Type'] = 'text/plain; charset=UTF-8; format=flowed';
  //$headers['Content-Type'] = 'text/html; charset=UTF-8;';

  // Add in the actual message
  $message[] = wordwrap($form_state['values']['message']);

  // Format the subject line:
  $subject = 'Nfl Pickem: '. $form_state['values']['subject'];

  // Prepare the body:
  $body = implode("\n\n", $message);

  // Load the recipients
  $recipients = array();
  foreach ($form_state['values']['league_id'] as $league_id) {
    $recipients = array_merge($recipients, get_users($league_id));
  }
  if (count($recipients) == 0) {
    drupal_set_message(t('There are no users in this league. Mail not sent.'));
    return '';
  }
  $recipients_str = '';
  foreach ($recipients as $r) {
    $recipients_str .= $r['name'] ." <". $r['mail'] .">,";
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
    drupal_set_message('[Success] Email(s) Sent');
  }
  else {
    drupal_set_message('[!Errors] errors encountered sending message. Please check the logs and try again.');  
  }

}














function page_admin_picks($league_id=0, $user_id = 0, $week_id = 0) {
  global $user;

  
  if ( is_numeric($_POST['league_select']) ) {
    drupal_goto("admin/content/pickem/picks/" . $_POST['league_select']);
  }
  if ( is_numeric($_POST['user_select']) ) {
    drupal_goto("admin/content/pickem/picks/$league_id/" . $_POST['user_select']);
  }

  if ( $league_id == 0 ) {
    drupal_set_title('Choose a League to administer');
    return drupal_get_form('league_select_form');
  }
  if ( $user_id == 0 ) {
    drupal_set_title('Choose a User to administer');
    return drupal_get_form('user_select_form', $league_id);
  }

  $week = get_weeks($week_id);
  $teams = get_teams();
  $u = user_load(array('uid' => $user_id));


  if ( $league_id != 0 && $user_id != 0 && $week_id != 0 && count($_POST) > 0 ) {
    
    //POSTING, save picks
      $type = "";
      foreach ($_POST as $gid => $pick) {
        $gid = preg_split('/_/', $gid);
        $query = 'select count(*) as pickcount from {pickem_picks} where gid=%d and lid=%d and uid=%d';
        $result = db_query($query, $gid[1], $league_id, $u->uid);
        if ($row = db_fetch_object($result)) {
          if ($row->pickcount > 0) {
            $type = "update";
          } 
          else {
            $type = "insert";
          }
        }
        if ($type == "insert") {
          $query = "insert into {pickem_picks} (gid,uid,lid,winnerpick_id,picktime) values (%d,%d,%d,%d,'%s')";
          $result = db_query($query, $gid[1], $u->uid, $league_id, $pick, date('Y-m-d H:i:s', time()));
        }
        elseif ($type == "update") {
          $query = "update {pickem_picks} set winnerpick_id=%d,picktime='%s' where gid=%d and uid=%d and lid=%d";
          $result = db_query($query, $pick, date('Y-m-d H:i:s', time()), $gid[1], $u->uid, $league_id);
        }
      }
      drupal_set_message("Picks Saved");
  }
  
  $l = get_leagues($league_id);

  // SINGLE WEEK
  if ( $week_id != 0 ) {
    drupal_set_title('Administer Picks - '. $l[$league_id]->title .' - '. $u->name ." - Week ". $week_id);
    
    // load up the games array
    $query = "SELECT g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, pp.winnerpick_id, g.gametime FROM {pickem_games} g LEFT OUTER JOIN {pickem_picks} pp ON pp.gid = g.gid and pp.lid=%d and pp.uid=%d where g.wid=%d ORDER BY g.wid, g.gametime, g.gid";
    $result = db_query($query, $league_id, $u->uid, $week_id);
    $games = array();
    while ($row = db_fetch_object($result)) {
      $games[$row->wid][$row->gid] = array(
        "hid" => $row->h_id,
        "vid" => $row->v_id,
        "pick" => $row->winnerpick_id,
        "gametime" => $row->gametime
      );
    }
    
    $header = array(t('Visitor'), t('Home Team'), t('Game Time(EST)') );
    $rows = array();
    
    foreach ($games as $wid => $weeksgames) {
      foreach ($weeksgames as $gid => $game) {
        $v_sel = "";
        $h_sel = "";

        $v_team = ($teams[$game["vid"]]->tm_abbrev <> '')?$teams[$game["vid"]]->tm_abbrev:$teams[$game["vid"]]->tm_name;
        $h_team = ($teams[$game["hid"]]->tm_abbrev <> '')?$teams[$game["hid"]]->tm_abbrev:$teams[$game["hid"]]->tm_name;
        
        if ($game["pick"] == $game["vid"]) {
          $v_sel = "checked";
        }
        elseif ($game["pick"] == $game["hid"]) {
          $h_sel = "checked";
        }
        
        if (isset($game["pick"])) {
          $class = 'pick';
        } 
        else {
          $class = 'nopick';
        }

        $rows[] = array(
          '<input type="radio" name="gid_'. $gid .'" value="'. $game["vid"] .'" '. $v_sel .'/>'. $v_team,
          '<input type="radio" name="gid_'. $gid .'" value="'. $game["hid"] .'" '. $h_sel .'/>'. $h_team,
          array('data' => $game['gametime'], 'class' => "$class"),
        );
      }
    }
    
    $output = '<div class="pickem"><form action="" method="post">';
    $output .= theme('table', $header, $rows);
    $output .= '<input type="submit" value="Save Picks"/>';
    $output .= '</form></div>';

  } 
  else {
    // ALL WEEKS, missing week_id, requires correct week start date
    drupal_set_title('Administer Picks - '. $l[$league_id]->title .' - '. $u->name);

    $flip = array('even' => 'odd', 'odd' => 'even');
    $class = 'even';
    $games = get_games_split($league_id, $user_id);
    foreach ($games as $wid => $weeksgames) {
      $inner="";
      foreach ($weeksgames as $gid => $game) {
        $class = $flip[$class];

        $v_team = ($teams[$game["vid"]]->tm_abbrev <> '')?$teams[$game["vid"]]->tm_abbrev:$teams[$game["vid"]]->tm_name;
        $h_team = ($teams[$game["hid"]]->tm_abbrev <> '')?$teams[$game["hid"]]->tm_abbrev:$teams[$game["hid"]]->tm_name;
        
        $inner .= '<div class="game-list-horiz">';
        if ( isset ($game["pick"]) ) {
          if ($game["pick"] == $game["vid"] && $game["vscore"] > $game["hscore"]) {
            // PICK VIS, VIS WINS
            $inner .= '<span class="win">'. $v_team .'</span><br/>@'. $h_team;
          }
          elseif ($game["pick"] == $game["hid"] && $game["hscore"] > $game["vscore"]) {
            // PICK HOME, HOME WINS
            $inner .= $v_team .'<br/>@<span class="win">'. $h_team .'</span>';
          }
          elseif ($game["hscore"] <> "" && $game["hscore"] == $game["vscore"]) {
            //TIE
            $inner .= '<span class="win">'. $v_team .'<br/>@'. $h_team .'</span>';
          }
          elseif ($game["pick"] == $game["vid"] && $game["vscore"] < $game["hscore"]) {
            // PICK VIS, VIS LOSES
            $inner .= '<span class="loss">'. $v_team .'</span><br/>@'. $h_team;
          }
          elseif ($game["pick"] == $game["hid"] && $game["hscore"] < $game["vscore"]) {
            // PICK HOME, HOME LOSES
            $inner .= $v_team .'<br/>@<span class="loss">'. $h_team .'</span>';
          }
          elseif ($game["pick"] == $game["hid"]) {
            // PICK HOME, GAME NOT PLAYED
            $inner .= $v_team .'<br/>@'. $h_team;
          }
          elseif ($game["pick"] == $game["vid"]) {
            // PICK VIS, GAME NOT PLAYED
            $inner .= $v_team .'<br/>@'. $h_team;
          }
        } 
        else {
          // NO PICK
          $inner .= $v_team .'<br/>@'. $h_team;
        }
        $inner .= "</div>";
      }
      $rows[] = array(
        l("$wid", "admin/content/pickem/picks/$league_id/$user_id/$wid"),
        $inner,
      );
    }
    $output = '<div class="pickem">Click on week number.';
    $output .= theme('table', array(), $rows);
    $output .= '</div>';
  }
  print theme('page', $output);
}


/*
  Path:     admin/content/pickem/scores
*/
function page_admin_scores($week_id = 0) {

  $week = get_weeks($week_id);
  $title = 'Pickem Scores Admin';
  //print_r($week);
  drupal_set_title($title);

  if (count($_POST) > 0) {

    $type = "";
    foreach ($_POST as $gid => $score) {
      $gid = preg_split('/_/', $gid);

      $query = "SELECT g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, g.gametime FROM {pickem_games} g where g.gid=%d ORDER BY g.wid, g.gametime, g.gid";
      $result = db_query($query, $gid[1]);
      $game = array();
      while ($row = db_fetch_object($result)) {
        $game = array(
          "hid" => $row->h_id,
          "vid" => $row->v_id,
          "vscore" => $row->v_score,
          "hscore" => $row->h_score,
          "gametime" => $row->gametime
        );
      }

      $query = "";
      if ($game["vid"] == $gid[2]) {
        $query = "update {pickem_games} set v_score=%d where gid=%d";
      } 
      elseif ($game["hid"] == $gid[2]) {
        $query = "update {pickem_games} set h_score=%d where gid=%d";
      }
      if ($score <> "" && $query <> "") {
        $result = db_query($query, $score, $gid[1]);
      }
    }
    cache_clear_all('pickem_grid_scores', 'cache', TRUE); 
    drupal_set_message("Scores Saved");
  }

  $teams = get_teams();
  
  if ( $week_id == 0 ) {
    // all weeks

    $games = get_games_split();
    $output = '<div class="pickem">Click on week number to update scores<table>';
    $flip = array('even' => 'odd', 'odd' => 'even');
    $class = 'even';

    foreach ($games as $wid => $weeksgames) {
      
      $class = $flip[$class];      
      $output .= '<tr class="'. $class .'"><th>'. l("$wid", 'admin/content/pickem/scores/'. $wid) .'</th><td>';
      foreach ($weeksgames as $gid => $game) {
        $v_team = ($teams[$game["vid"]]->tm_abbrev <> '')?$teams[$game["vid"]]->tm_abbrev:$teams[$game["vid"]]->tm_name;
        $h_team = ($teams[$game["hid"]]->tm_abbrev <> '')?$teams[$game["hid"]]->tm_abbrev:$teams[$game["hid"]]->tm_name;
        $output .= '<div class="game-list-horiz">'. $v_team .'&nbsp;'. $game["vscore"] .'<br>'. $h_team .'&nbsp;'. $game["hscore"] .'</div>';
      }
      $output .= "</td></tr>";
    }
    $output .= '</table></div>';
  } 
  else {
    drupal_set_title("$title - Week $week_id");
    $query = "SELECT g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, g.gametime FROM {pickem_games} g where g.wid=%d ORDER BY g.wid, g.gametime, g.gid";
    $result = db_query($query, $week_id);
    $games = array();
    while ($row = db_fetch_object($result)) {
      $games[$row->wid][$row->gid] = array(
        "hid" => $row->h_id,
        "vid" => $row->v_id,
        "vscore" => $row->v_score,
        "hscore" => $row->h_score,
        "gametime" => $row->gametime
      );
    }
    $output = '<div class="pickem"><form action="" method="post"><table class="admin-scores">';
    $output .= '<tr><th colspan="2">Visitor</th><th colspan="2">Home</th><th>Game Time(EST)</th></tr>';
    foreach ($games as $wid => $weeksgames) {
      foreach ($weeksgames as $gid => $game) {
        $v_team = ($teams[$game["vid"]]->tm_abbrev <> '')?$teams[$game["vid"]]->tm_abbrev:$teams[$game["vid"]]->tm_name;
        $h_team = ($teams[$game["hid"]]->tm_abbrev <> '')?$teams[$game["hid"]]->tm_abbrev:$teams[$game["hid"]]->tm_name;
        
        $output .= '<tr>';
        $output .= '<td class="team">'. $v_team .'</td><td class="score"><input name="gid_'. $gid .'_'. $game['vid'] .'" type="text" size="3" value="'. $game['vscore'] .'" /></td>';
        $output .= '<td class="team">'. $h_team .'</td><td class="score"><input name="gid_'. $gid .'_'. $game['hid'] .'" type="text" size="3" value="'. $game['hscore'] .'" /></td>';
        $output .= '<td class="time">'. $game['gametime'] .'</td></tr>';
      }
    }
    $output .= '</table>';
    $output .= '<input type="submit" value="savescores"/>';
    $output .= '</form></div>';
  }

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




/*
  Admin Games
*/
function page_admin_data_import() {
  drupal_set_title('Pickem Import Data');
  return drupal_get_form('admin_import_form');
}
function admin_import_form($form_state=NULL) {
  $form['pickem_data_load'] = array(
    '#type' => 'fieldset',
    '#title' => t('Initial Data Load'),
    '#description' => t('WARNING: This will clear out previously stored data.'),
    '#collapsible' => TRUE,
  );
  $form['pickem_data_load']['pickem_data_load_options'] = array(
    '#type' => 'radios',
    '#title' => t('Choose data to load'),
    '#options' => array(0 => t('2008 NFL Data'), 1 => t('2008 NCAA Div 1, Basketball')),
    '#description' => t('This will delete existing teams, games, and picks.'),
  );

  $form['pickem_data_load']['pickem_load_data_button'] = array(
    '#type' => 'submit',
    '#value' => t('Load data now'),
  );

  return $form; 
}

function admin_import_form_submit($form, &$form_state) {
  
  if ( $form_state['values']['pickem_data_load_options'] == 0 ) {
    clear_league_data();
    load_nfl_teams_csv();
    load_nfl_weeks_csv();
    load_nfl_games_csv();
    drupal_set_message("NFL data loaded");
  }
  elseif ( $form_state['values']['pickem_data_load_options'] == 1 ) {
    clear_league_data();
    load_ncaa_basketball_teams_csv();
  }
  else {
    drupal_set_message("Data Import NOT Completed.");
  }
return;
}

function clear_league_data() {
  if ( db_query("delete from {pickem_teams}") == FALSE ) {
    drupal_set_message("Couldn't Delete Teams.", 'error');
  }
  if ( db_query("delete from {pickem_weeks}") == FALSE ) {
    drupal_set_message("Couldn't Delete Weeks.", 'error');
  }
  if ( db_query("delete from {pickem_games}") == FALSE ) {
    drupal_set_message("Couldn't Delete Games.", 'error');
  }
}

function load_nfl_teams_csv() {
  // keep fetching a line from the file until end of file
  $fp = fopen(drupal_get_path('module', 'pickem') ."/data/pickem_teams.csv", "r");
  $row = 1;
  $field_names = array();
  while (($data = fgetcsv($fp, 1024, ';')) !== FALSE) {
    $num = count($data);
    if ( $row == 1 ) {
      for ($c=0; $c < $num; $c++) {
        $field_names[$data[$c]] = $c;
      }
    } 
    else {
      db_query("INSERT INTO {pickem_teams} (tid, tm_city, tm_abbrev, tm_nick, tm_name, conference, division) VALUES (%d, '%s', '%s', '%s', '%s', '%s', '%s')", $data[$field_names['tid']], $data[$field_names['tm_city']], $data[$field_names['tm_abbrev']], $data[$field_names['tm_nick']], $data[$field_names['tm_name']], $data[$field_names['conference']], $data[$field_names['division']] );
    }
    $row++;
  }
  fclose($fp);
}

function load_nfl_weeks_csv() {
  // keep fetching a line from the file until end of file
  $fp = fopen(drupal_get_path('module', 'pickem') ."/data/pickem_weeks.csv", "r");
  $row = 1;
  $field_names = array();
  while (($data = fgetcsv($fp, 1024, ';')) !== FALSE) {
    $num = count($data);
    if ( $row == 1 ) {
      for ($c=0; $c < $num; $c++) {
        $field_names[$data[$c]] = $c;
      }
    } 
    else {
      db_query("INSERT INTO {pickem_weeks} (wid, wk_name, wk_abbrev, wk_firstgame, wk_points, playoffs) VALUES (%d, '%s', '%s', '%s', %d, %d)", $data[$field_names['wid']], $data[$field_names['wk_name']], $data[$field_names['wk_abbrev']], $data[$field_names['wk_firstgame']], $data[$field_names['wk_points']], $data[$field_names['playoffs']] );
    }
    $row++;
  }
  fclose($fp);
}

function load_nfl_games_csv() {
  // keep fetching a line from the file until end of file
  $fp = fopen(drupal_get_path('module', 'pickem') ."/data/pickem_games.csv", "r");
  $row = 1;
  $field_names = array();
  while (($data = fgetcsv($fp, 1024, ';')) !== FALSE) {
    $num = count($data);
    if ( $row == 1 ) {
      for ($c=0; $c < $num; $c++) {
        $field_names[$data[$c]] = $c;
      }
    } 
    else {
      //drupal_set_message("wid=".$data[$field_names['wid']] . ' gametime=' . $data[$field_names['gametime']] . ' h_id=' . $data[$field_names['h_id']]  . ' v_id=' . $data[$field_names['v_id']] ) ;
      db_query("INSERT INTO {pickem_games} (wid, gametime, h_id, v_id) VALUES (%d, '%s', %d, %d)", $data[$field_names['wid']], $data[$field_names['gametime']], $data[$field_names['h_id']], $data[$field_names['v_id']] );
    }
    $row++;
  }
  fclose($fp);
}

function load_ncaa_basketball_teams_csv() {
  $path = drupal_get_path('module', 'pickem') .'/data/ncaa_div1_basketball_teams.csv';
  $fp = fopen($path, "r");
  if ( $fp == FALSE ) {
    drupal_set_message("Load failed for Teams: 2008 NCAA Div 1, Basketball", 'error');
    return FALSE;
  }

  $row = 1;
  $field_names = array();
  while (($data = fgetcsv($fp, 1024, ';')) !== FALSE) {
    $num = count($data);
    if ( $row == 1 ) {
      for ($c=0; $c < $num; $c++) {
        $field_names[$data[$c]] = $c;
      }
    }
    else {
      if ( db_query("INSERT INTO {pickem_teams} (tid, tm_city, tm_abbrev, tm_nick, tm_name, conference, division) VALUES (%d, '%s', '%s', '%s', '%s', '%s', '%s')", $data[$field_names['tid']], $data[$field_names['tm_city']], $data[$field_names['tm_abbrev']], $data[$field_names['tm_nick']], $data[$field_names['tm_name']], $data[$field_names['conference']], $data[$field_names['division']]) == FALSE ) {
        drupal_set_message("Couldn't Load row=$row for Teams: 2008 NCAA Div 1, Basketball", 'error');
      }
    }
    $row++;
  }
  fclose($fp);
  drupal_set_message("Loaded Teams: 2008 NCAA Div 1, Basketball");
  return TRUE;
}