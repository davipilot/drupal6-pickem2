<?php
// $Id: pickem_admin.inc,v 1.1.2.3 2008/09/22 18:20:32 jvandervort Exp $

/**
 * @file
 * includes all pickem admin functions.
*/

function pickem_admin_settings() {

  $form = array();

  $form['pickem_commisioner_email'] = array(
    '#type' => 'textfield',
    '#size' => 100,
    '#title' => t('Email Address of the Commisioner'),
    '#default_value' => variable_get('pickem_commisioner_email', ''),
    '#description' => t("This is the return address for all leaguewide pickem email."),
  );

  $form['pickem_use_espn'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use the ESPN score scrape'),
    '#return_value' => 1,
    '#default_value' => variable_get('pickem_use_espn', 0),
    '#description' => t("This grabs the current weeks scores from espn."),
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
 
  drupal_set_title('Administer League - '. $league->title);
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
    $team_list[$t_id] = $team_data->tm_abbrev;
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

  foreach ($form_state['values']['games'] as $key => $value) {
    $gid = $value['gid'];
    $wid = $value['wid'];
    $gametime = $value['game_time'];
    $home_team = $value['home_team'];
    $h_spread = $value['h_spread'];
    $visitor_team = $value['visitor_team'];
    $v_spread = $value['v_spread'];

    // this will filter by games on the current page only.
    if ($gid <> 0 && $gametime != '' && $home_team <> 0 && $visitor_team <> 0) {
      $query = "update {pickem_games} set wid=%d, gametime='%s', h_id=%d, h_spread=%d, v_id=%d, v_spread=%d where gid=%d";
      $result = db_query($query, $wid, date($gametime), $home_team, $h_spread, $visitor_team, $v_spread, $gid);
    }
  }

  foreach ($form_state['values']['new_game'] as $key => $value) {
    $gid = $value['gid'];
    $wid = $value['wid'];
    $gametime = $value['game_time'];
    $home_team = $value['home_team'];
    $h_spread = $value['h_spread'];
    $visitor_team = $value['visitor_team'];
    $v_spread = $value['v_spread'];

    if ($gid == 0 && $gametime != '' && $home_team <> 0 && $visitor_team <> 0) {
      $query = "insert into {pickem_games}(wid, gametime, h_id, h_spread, v_id, v_spread) values(%d, '%s', %d, %d, %d, %d)";
      $result = db_query($query, $wid, date($gametime), $home_team, $h_spread, $visitor_team, $v_spread);
      drupal_set_message("Games Saved");
      drupal_goto('admin/content/pickem/games', 'page='. ($wid - 1));
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
  
  $query = "SELECT wid, wk_name, wk_abbrev, wk_firstgame, wk_points from {pickem_weeks} ORDER BY wid";
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
      )
    );
  }
  // add new week
  $form['weeks'][] = array(
    'wid' => array(
      '#type' => 'hidden',
      '#value' => 0
    ),
    'num' => array(
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
    );
  }

  $output = theme('table', $header, $rows);

  $output .= drupal_render($form['save']);
  $output .= drupal_render($form['form_id']);
  $output .= drupal_render($form['form_token']);

  return $output;
}

function admin_weeks_form_submit($form, &$form_state) {
  //drupal_set_message('admin_weeks_form_submit');
  
  foreach ($form_state['values']['weeks'] as $key => $value) {
    $wid = $value['wid'];
    $num = $value['num'];
    $name = $value['name'];
    $abbrev = $value['abbrev'];
    $first_game = $value['firstgame'];
    $points = $value['points'];

    if ($wid == 0 && $num <> "") {
      $query = "insert into {pickem_weeks}(wid, wk_name, wk_abbrev, wk_firstgame, wk_points) values(%d, '%s', '%s', '%s', %d)";
      $result = db_query($query, $num, $name, $abbrev, date($first_game), $points);
    } 
    else {
      $query = "update {pickem_weeks} set wid=%d, wk_name='%s', wk_abbrev='%s', wk_firstgame='%s', wk_points=%d where wid=%d";
      $result = db_query($query, $num, $name, $abbrev, date($first_game), $points, $wid);
    }
  }
  cache_clear_all('pickem_grid_scores', 'cache', TRUE); 
  drupal_set_message("Weeks Saved");
}



/*
  Admin Teams
*/
function page_admin_teams() {
  drupal_set_title('Pickem Teams Admin');
  return drupal_get_form('admin_teams_form');
}
function admin_teams_form($form_state=NULL) {
  //drupal_set_message("admin_teams_form");

  $form = array(
    '#theme' => 'admin_teams_form',
    '#tree' => TRUE,
  );  

  $query = "SELECT tid, tm_city, tm_abbrev, tm_nick, tm_name, conference, division from {pickem_teams} order by tid";
  $result = db_query($query);
  while ($row = db_fetch_object($result)) {
    $form['teams'][$row->tid] = array(
      'rowid' => array(
        '#type' => 'value',
        '#value' => $row->tid
      ),
      'tid' => array(
        '#type' => 'textfield',
        '#size' => 3,
        '#default_value' => $row->tid
      ),
      'city' => array(
        '#type' => 'textfield',
        '#size' => 20,
        '#default_value' => $row->tm_city
      ),
      'abbrev' => array(
        '#type' => 'textfield',
        '#size' => 3,
        '#maxlength' => 3,
        '#default_value' => $row->tm_abbrev
      ),
      'nick' => array(
        '#type' => 'textfield',
        '#size' => 35,
        '#default_value' => $row->tm_nick
      ),
      'name' => array(
        '#type' => 'textfield',
        '#size' => 15,
        '#default_value' => $row->tm_name
      ),
      'conf' => array(
        '#type' => 'textfield',
        '#size' => 10,
        '#default_value' => $row->conference
      ),
      'div' => array(
        '#type' => 'textfield',
        '#size' => 10,
        '#default_value' => $row->division
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
  //drupal_set_message("theme_admin_teams_form");
  
  $header = array(t('TID'), t('Abbrev'), t('City'), t('Name'), t('Nickname'), t('Conference'), t('Division'));
  //$destination = drupal_get_destination();

  $rows = array();
  foreach (element_children($form['teams']) as $key) {
    $rows[] = array(
      drupal_render($form['teams'][$key]['tid']), 
      drupal_render($form['teams'][$key]['abbrev']), 
      drupal_render($form['teams'][$key]['city']), 
      drupal_render($form['teams'][$key]['name']), 
      drupal_render($form['teams'][$key]['nick']),
      drupal_render($form['teams'][$key]['conf']),
      drupal_render($form['teams'][$key]['div']),
    );
  }
 
  $output = theme('table', $header, $rows);
  $output .= drupal_render($form['save']);
  $output .= drupal_render($form['form_id']);
  $output .= drupal_render($form['form_token']);
  
  return $output;
}

function admin_teams_form_submit($form, &$form_state) {
  //drupal_set_message("admin_teams_form_submit");
  
  foreach ($form_state['values']['teams'] as $key => $value) {
    $tid    = $value['tid'];
    $city   = $value['city'];
    $abbrev = $value['abbrev'];
    $name   = $value['name'];
    $nick   = $value['nick'];
    $conf   = $value['conf'];
    $div   = $value['div'];
    
    $query = "update {pickem_teams} set tm_city='%s', tm_abbrev='%s', tm_name='%s', tm_nick='%s', conference='%s', division='%s' where tid=%d";
    $result = db_query($query, $city, $abbrev, $name, $nick, $conf, $div, $tid);
  }
  drupal_set_message("Teams Saved");
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


/**************
  Admin Leagues
***************/
function page_admin_leagues() {
  drupal_set_title('Pickem Leagues Admin');
  return drupal_get_form('admin_leagues_form');
}
function admin_leagues_form($form_state=NULL) {
  //drupal_set_message("admin_leagues_form");

  $form = array(
    '#theme' => 'admin_leagues_form',
    '#tree' => TRUE,
  );  

  $query = "SELECT * from {pickem_leagues} order by lid";
  $result = db_query($query);
  $league_forums_options = league_forums();
  while ($row = db_fetch_object($result)) {
    $form['leagues'][$row->lid] = array(
      'lid' => array(
        '#type' => 'hidden',
        '#value' => $row->lid
      ),
      'name' => array(
        '#type' => 'textfield',
        '#size' => 50,
        '#default_value' => $row->name
      ),
      'forum_id' => array(
        '#type' => 'select',
        '#default_value' => $row->forum_id,
        '#options' => $league_forums_options,
      ),
      'rules_node_id' => array(
        '#type' => 'textfield',
        '#size' => 5,
        '#default_value' => $row->rules_node_id
      ),
      'scoring_type' => array(
        '#type' => 'radios',
        '#default_value' => $row->scoring_type,
        '#options' => array( 0 => t('Win-Loss'), 1 => t('Spread') ),
      ),
    );
  }
  
    $form['add_league'] = array(
      'lid' => array(
        '#type' => 'hidden',
        '#value' => 0
      ),
      'name' => array(
        '#type' => 'textfield',
        '#size' => 50,
        '#default_value' => ''
      ),
      'forum_id' => array(
        '#type' => 'select',
        '#default_value' => 0,
        '#options' => $league_forums_options,
      ),
      'rules_node_id' => array(
        '#type' => 'textfield',
        '#size' => 5,
        '#default_value' => NULL
      ),
      'scoring_type' => array(
        '#type' => 'radios',
        '#default_value' => $row->scoring_type,
        '#options' => array( 0 => t('Win-Loss'), 1 => t('Spread') ),
      ),
    );
  
  
  
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save Leagues')
  );

  return $form;
}
function theme_admin_leagues_form(&$form) {
  //drupal_set_message("theme_admin_leagues_form");
  
  drupal_add_js('misc/collapse.js');
  
  $header = array(t('Name'), t('Chat Forum'), t('Rules Node ID') , t('Scoring Type'));

  if ( count(element_children($form['leagues'])) > 0 ) {
    $rows = array();
    foreach (element_children($form['leagues']) as $key) {
      $rows[] = array(
        drupal_render($form['leagues'][$key]['lid']) . drupal_render($form['leagues'][$key]['name']), 
        drupal_render($form['leagues'][$key]['forum_id']), 
        drupal_render($form['leagues'][$key]['rules_node_id']), 
        drupal_render($form['leagues'][$key]['scoring_type']), 
      );
    }
    
    $output = '<div><fieldset class="collapsible"><legend><a href="#">Leagues</a></legend>';
    $output .= theme('table', $header, $rows);
    $output .= "</fieldset></div>";
  }

  $new_row[] = array(
    drupal_render($form['add_league']['lid']) . drupal_render($form['add_league']['name']), 
    drupal_render($form['add_league']['forum_id']), 
    drupal_render($form['add_league']['rules_node_id']), 
    drupal_render($form['add_league']['scoring_type']), 
  );

  $output .= '<div><fieldset class="collapsible"><legend><a href="#">Add New League</a></legend>';
  $output .= theme('table', $header, $new_row);
  $output .= "</fieldset></div>";

  $output .= drupal_render($form['save']);
  $output .= drupal_render($form['form_id']);
  $output .= drupal_render($form['form_token']);
  
  return $output;
}
function admin_leagues_form_submit($form, &$form_state) {
  //drupal_set_message("admin_leagues_form_submit");
  
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
          '<input type="radio" name="gid_'. $gid .'" value="'. $game["vid"] .'" '. $v_sel .'/>'. $teams[$game["vid"]]->tm_abbrev,
          '<input type="radio" name="gid_'. $gid .'" value="'. $game["hid"] .'" '. $h_sel .'/>'. $teams[$game["hid"]]->tm_abbrev,
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
        
        $inner .= '<div class="game-list-horiz">';
        if (isset ($teams[$game["pick"]]->tm_abbrev)) {
          if ($game["pick"] == $game["vid"] && $game["vscore"] > $game["hscore"]) {
            // PICK VIS, VIS WINS
            $inner .= '<span class="win">'. $teams[$game["vid"]]->tm_abbrev .'</span><br/>@'. $teams[$game["hid"]]->tm_abbrev;
          }
          elseif ($game["pick"] == $game["hid"] && $game["hscore"] > $game["vscore"]) {
            // PICK HOME, HOME WINS
            $inner .= $teams[$game["vid"]]->tm_abbrev .'<br/>@<span class="win">'. $teams[$game["hid"]]->tm_abbrev .'</span>';
          }
          elseif ($game["hscore"] <> "" && $game["hscore"] == $game["vscore"]) {
            //TIE
            $inner .= '<span class="win">'. $teams[$game["vid"]]->tm_abbrev .'<br/>@'. $teams[$game["hid"]]->tm_abbrev .'</span>';
          }
          elseif ($game["pick"] == $game["vid"] && $game["vscore"] < $game["hscore"]) {
            // PICK VIS, VIS LOSES
            $inner .= '<span class="loss">'. $teams[$game["vid"]]->tm_abbrev .'</span><br/>@'. $teams[$game["hid"]]->tm_abbrev;
          }
          elseif ($game["pick"] == $game["hid"] && $game["hscore"] < $game["vscore"]) {
            // PICK HOME, HOME LOSES
            $inner .= $teams[$game["vid"]]->tm_abbrev .'<br/>@<span class="loss">'. $teams[$game["hid"]]->tm_abbrev .'</span>';
          }
          elseif ($game["pick"] == $game["hid"]) {
            // PICK HOME, GAME NOT PLAYED
            $inner .= $teams[$game["vid"]]->tm_abbrev .'<br/>@'. $teams[$game["hid"]]->tm_abbrev;
          }
          elseif ($game["pick"] == $game["vid"]) {
            // PICK VIS, GAME NOT PLAYED
            $inner .= $teams[$game["vid"]]->tm_abbrev .'<br/>@'. $teams[$game["hid"]]->tm_abbrev;
          }
        } 
        else {
          // NO PICK
          $inner .= $teams[$game["vid"]]->tm_abbrev .'<br/>@'. $teams[$game["hid"]]->tm_abbrev;
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
        $output .= '<div class="game-list-horiz">'. $teams[$game["vid"]]->tm_abbrev .'&nbsp;'. $game["vscore"] .'<br>'. $teams[$game["hid"]]->tm_abbrev .'&nbsp;'. $game["hscore"] .'</div>';
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
        $output .= '<tr>';
        $output .= '<td class="team">'. $teams[$game["vid"]]->tm_abbrev .'</td><td class="score"><input name="gid_'. $gid .'_'. $game['vid'] .'" type="text" size="3" value="'. $game['vscore'] .'" /></td>';
        $output .= '<td class="team">'. $teams[$game["hid"]]->tm_abbrev .'</td><td class="score"><input name="gid_'. $gid .'_'. $game['hid'] .'" type="text" size="3" value="'. $game['hscore'] .'" /></td>';
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
  $sql = db_rewrite_sql("SELECT distinct td.tid, td.name FROM {term_data} td inner join {forum} f on f.tid=td.tid  ORDER BY td.name");
  $result = db_query($sql);
  $options = array();
  $options[0] = 'No Forum Selected';
  while ($row = db_fetch_object($result)) {
    $options[$row->tid] = $row->name; 
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
  $form['pickem_data_load']['pickem_data_load_confirm'] = array(
    '#type' => 'checkbox',
    '#title' => t('Load 2008 NFL Data'),
    '#return_value' => 1,
    '#default_value' => 0,
    '#description' => t('This will reload teams, games, and picks.'),
  );
  $form['pickem_data_load']['pickem_load_data_button'] = array(
    '#type' => 'submit',
    '#value' => t('Load data now'),
    '#submit' => array('pickem_data_load_submit'),
  );

  return $form; 
}

function pickem_data_load_submit($form, &$form_state) {
  
  //print_r($form_state);
  //return;

  if ( $form_state['values']['pickem_data_load_confirm'] == 1 ) {
    load_teams_csv();
    load_weeks_csv();
    load_games_csv();
    drupal_set_message("Data Import Completed");
  }
  else {
    drupal_set_message("Data Import NOT Completed.  Select 'Load Data' checkbox.");
  }
return;
}
function load_teams_csv() {
  
  db_query("delete from {pickem_teams}");

  // keep fetching a line from the file until end of file
  $fp = fopen(drupal_get_path('module', 'pickem') ."/data/pickem_teams.csv", "r");
  $row = 1;
  $field_names = array();
  while (($data = fgetcsv($fp, 0, ';')) !== FALSE) {
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

function load_weeks_csv() {
  
  db_query("delete from {pickem_weeks}");

  // keep fetching a line from the file until end of file
  $fp = fopen(drupal_get_path('module', 'pickem') ."/data/pickem_weeks.csv", "r");
  $row = 1;
  $field_names = array();
  while (($data = fgetcsv($fp, 0, ';')) !== FALSE) {
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

function load_games_csv() {
  
  db_query("delete from {pickem_games}");
  
  // keep fetching a line from the file until end of file
  $fp = fopen(drupal_get_path('module', 'pickem') ."/data/pickem_games.csv", "r");
  $row = 1;
  $field_names = array();
  while (($data = fgetcsv($fp, 0, ';')) !== FALSE) {
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
