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
    '#description' => t("This is the return address for all systemwide email messages."),
  );

  return system_settings_form($form);
}





function page_admin_leagues($league_id = "") {
  
  if ( is_numeric($_POST['league_select']) ) {
    drupal_goto("admin/content/pickem/league/" . $_POST['league_select']);
  }
  
  drupal_set_title('Choose a League to administer');
  return drupal_get_form('league_select_form');
}


function page_admin_league($league) {

  /* 
    Post Actions
  */
  if ( $_POST['user_select'] && $_POST['op'] == t('Add') ) {
  // user_select
    $u = user_load(array('uid' => $_POST['user_select']));
    db_query("insert into {pickem_users} (nid,uid) values (%d,%d)", $league->lid, $u->uid);
    drupal_set_message($u->name .' added to '. $league->title);
    drupal_goto("admin/content/pickem/league/". $league->lid);
  }

  if ( arg(5) == "delete_user" && is_numeric(arg(6)) ) {
    $u = user_load(array('uid' => arg(6)));
    db_query("delete from {pickem_users} where nid=%d and uid=%d", $league->lid, $u->uid);
    drupal_set_message($u->name .' removed from '. $league->title);
    drupal_goto("admin/content/pickem/league/". $league->lid);
  }


  /*
    Page Display
  */
  $header = array(t('Name'), t('Operations'));
  $rows = array();
  
  $query = "SELECT u.uid,name from {users} u inner join {pickem_users} pu on pu.uid=u.uid where u.uid<>0 and nid=%d order by u.uid";
  $result = db_query($query, $league->lid);
  while ($row = db_fetch_object($result)) {
    $rows[] = array(l($row->name, 'user/'. $row->uid), l(t('delete user'), 'admin/content/pickem/league/'. $league->lid .'/delete_user/'. $row->uid));
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
    $team_list[$t_id] = $team_data["abbrev"];
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
      'visitor_team' => array(
        '#type' => 'select',
        '#default_value' => $game_data->v_id,
        '#options' => $team_list,
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
    'visitor_team' => array(
      '#type' => 'select',
      '#default_value' => "",
      '#options' => $team_list,
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
        drupal_render($form['games'][$key]['home_team'])
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
      drupal_render($form['new_game'][$key]['home_team'])
    );
  }

  $header = array(t('Week'), t('Time'), t('Visitor'), t('Home'));
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
    $visitor_team = $value['visitor_team'];

    // this will filter by games on the current page only.
    if ($gid <> 0 && $gametime != '' && $home_team <> 0 && $visitor_team <> 0) {
      $query = "update {pickem_games} set wid=%d, gametime='%s', h_id=%d, v_id=%d where gid=%d";
      $result = db_query($query, $wid, date($gametime), $home_team, $visitor_team, $gid);
    }
  }

  foreach ($form_state['values']['new_game'] as $key => $value) {
    $gid = $value['gid'];
    $wid = $value['wid'];
    $gametime = $value['game_time'];
    $home_team = $value['home_team'];
    $visitor_team = $value['visitor_team'];

    if ($gid == 0 && $gametime != '' && $home_team <> 0 && $visitor_team <> 0) {
      $query = "insert into {pickem_games}(wid, gametime, h_id, v_id) values(%d, '%s', %d, %d)";
      $result = db_query($query, $wid, date($gametime), $home_team, $visitor_team);
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
  $leagues = array();
  
  $sql = "SELECT n.title, n.nid FROM {node} n WHERE n.type ='league' AND n.status = 1 ORDER BY n.title ASC";
  $result = db_query($sql, $types);
  while ($row = db_fetch_object($result)) {
    $leagues[$row->nid] = $row->title;
  }
  
  $form['league_select'] = array( '#type' => 'select', '#title' => t('Select League'), '#default_value' => 1, '#options' => $leagues);
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
  foreach ($all_leagues as $l_id => $l_data) {
    $leagues[$l_id] = $l_data->title;
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
  global $user;

  $bcc = $form_state['values']['bcc'];
  $from = $user->name ."<". $user->mail .">";

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
    $recipients = $recipients + get_users($league_id);
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

  $message = drupal_mail('pickem', 'notice', $to, language_default(), $params, $from);
  if ($message['result'] == 1) {
    drupal_set_message('[Success] Email(s) Sent');
  }
  else {
    drupal_set_message('[!Errors] errors encountered sending message. Please check the logs and try again.');  
  }

}













function ______UPDATE_DIVIDER______() {}


















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
        $query = 'select count(*) as pickcount from {pickem_picks} where gid=%d and nid=%d and uid=%d';
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
          $query = "insert into {pickem_picks} (gid,uid,nid,winnerpick_id,picktime) values (%d,%d,%d,%d,'%s')";
          $result = db_query($query, $gid[1], $u->uid, $league_id, $pick, date('Y-m-d H:i:s', time()));
        }
        elseif ($type == "update") {
          $query = "update {pickem_picks} set winnerpick_id=%d,picktime='%s' where gid=%d and uid=%d and nid=%d";
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
    $query = "SELECT g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, pp.winnerpick_id, g.gametime FROM {pickem_games} g LEFT OUTER JOIN {pickem_picks} pp ON pp.gid = g.gid and pp.nid=%d and pp.uid=%d where g.wid=%d ORDER BY g.wid, g.gametime, g.gid";
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
          '<input type="radio" name="gid_'. $gid .'" value="'. $game["vid"] .'" '. $v_sel .'/>'. $teams[$game["vid"]]["abbrev"],
          '<input type="radio" name="gid_'. $gid .'" value="'. $game["hid"] .'" '. $h_sel .'/>'. $teams[$game["hid"]]["abbrev"],
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

    $games = array();
    $query = "SELECT g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, pp.winnerpick_id, g.gametime FROM {pickem_games} g LEFT OUTER JOIN {pickem_picks} pp ON pp.gid = g.gid and pp.nid=%d and pp.uid=%d inner join {pickem_weeks} w on w.wid=g.wid where now() < adddate(w.wk_firstgame, INTERVAL 6 day) ORDER BY g.wid, g.gametime, g.gid";
    $result = db_query($query, $league_id, $u->uid);
    while ($row = db_fetch_object($result)) {
      $games[$row->wid][$row->gid] = array(
        "hid" => $row->h_id,
        "vid" => $row->v_id,
        "pick" => $row->winnerpick_id,
        "vscore" => $row->v_score,
        "hscore" => $row->h_score,
        "gametime" => $row->gametime
      );
    }

    $query = "SELECT g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, pp.winnerpick_id, g.gametime FROM {pickem_games} g LEFT OUTER JOIN {pickem_picks} pp ON pp.gid = g.gid and pp.nid=%d and pp.uid=%d inner join {pickem_weeks} w on w.wid=g.wid where now() > adddate(w.wk_firstgame, INTERVAL 6 day) ORDER BY g.wid, g.gametime, g.gid";
    $result = db_query($query, $league_id, $u->uid);
    while ($row = db_fetch_object($result)) {
      $games[$row->wid][$row->gid] = array(
        "hid" => $row->h_id,
        "vid" => $row->v_id,
        "pick" => $row->winnerpick_id,
        "vscore" => $row->v_score,
        "hscore" => $row->h_score,
        "gametime" => $row->gametime
      );
    }
    
    $rows = array();
    foreach ($games as $wid => $weeksgames) {
      $inner="";
      foreach ($weeksgames as $gid => $game) {
        $inner .= '<div style="float:left;margin:5px 0 0 3px;width:60px;border-left: 2px solid black;">';
        if (isset ($teams[$game["pick"]]["abbrev"])) {
          if ($game["pick"] == $game["vid"] && $game["vscore"] > $game["hscore"]) {
            // PICK VIS, VIS WINS
            $inner .= '<span class="pick_win"><b>'. $teams[$game["vid"]]["abbrev"] .'</b></span><br/>@'. $teams[$game["hid"]]["abbrev"];
          }
          elseif ($game["pick"] == $game["hid"] && $game["hscore"] > $game["vscore"]) {
            // PICK HOME, HOME WINS
            $inner .= $teams[$game["vid"]]["abbrev"] .'<br/>@<span class="pick_win"><b>'. $teams[$game["hid"]]["abbrev"] .'</b></span>';
          }
          elseif ($game["hscore"] <> "" && $game["hscore"] == $game["vscore"]) {
            //TIE
            $inner .= '<span class="pick_win">'. $teams[$game["vid"]]["abbrev"] .'<br/>@'. $teams[$game["hid"]]["abbrev"] .'</span>';
          }
          elseif ($game["pick"] == $game["vid"] && $game["vscore"] < $game["hscore"]) {
            // PICK VIS, VIS LOSES
            $inner .= '<span class="pick_loss"><b>'. $teams[$game["vid"]]["abbrev"] .'</b></span><br/>@'. $teams[$game["hid"]]["abbrev"];
          }
          elseif ($game["pick"] == $game["hid"] && $game["hscore"] < $game["vscore"]) {
            // PICK HOME, HOME LOSES
            $inner .= $teams[$game["vid"]]["abbrev"] .'<br/>@<span class="pick_loss"><b>'. $teams[$game["hid"]]["abbrev"] .'</b></span>';
          }
          elseif ($game["pick"] == $game["hid"]) {
            // PICK HOME, GAME NOT PLAYED
            $inner .= $teams[$game["vid"]]["abbrev"] .'<br/>@<b>'. $teams[$game["hid"]]["abbrev"] .'</b>';
          }
          elseif ($game["pick"] == $game["vid"]) {
            // PICK VIS, GAME NOT PLAYED
            $inner .= '<b>'. $teams[$game["vid"]]["abbrev"] .'</b><br/>@'. $teams[$game["hid"]]["abbrev"];
          }
        } 
        else {
          // NO PICK
          $inner .= $teams[$game["vid"]]["abbrev"] .'<br/>@'. $teams[$game["hid"]]["abbrev"];
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
function page_admin_scores($weekarg = "") {

  $week = get_weeks($weekarg);
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

  if (!is_numeric($weekarg)) {
    // all weeks
    $games = array();
    
    $query = "SELECT g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, g.gametime FROM {pickem_games} g inner join {pickem_weeks} w on w.wid=g.wid where now() < adddate(w.wk_firstgame, INTERVAL 6 day) ORDER BY g.wid, g.gametime, g.gid";
    $result = db_query($query);
    while ($row = db_fetch_object($result)) {
      $games[$row->wid][$row->gid] = array(
        "hid" => $row->h_id,
        "vid" => $row->v_id,
        "vscore" => $row->v_score,
        "hscore" => $row->h_score,
        "gametime" => $row->gametime
      );
    }

    $query = "SELECT g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, g.gametime FROM {pickem_games} g inner join {pickem_weeks} w on w.wid=g.wid where now() > adddate(w.wk_firstgame, INTERVAL 6 day) ORDER BY g.wid, g.gametime, g.gid";
    $result = db_query($query);
    while ($row = db_fetch_object($result)) {
      $games[$row->wid][$row->gid] = array(
        "hid" => $row->h_id,
        "vid" => $row->v_id,
        "vscore" => $row->v_score,
        "hscore" => $row->h_score,
        "gametime" => $row->gametime
      );
    }





    $output = '<div class="pickem">Click on week number to update scores<table>';
    foreach ($games as $wid => $weeksgames) {
      $output .= '<tr><th>'. l("$wid", 'admin/content/pickem/scores/'. $wid) .'</th><td>';
      foreach ($weeksgames as $gid => $game) {
        $output .= '<div style="float:left;margin:5px 0 0 3px;width:60px;border-left: 2px solid black;">'. $teams[$game["vid"]]["abbrev"] .'&nbsp;<b>'. $game["vscore"] .'</b><br>'. $teams[$game["hid"]]["abbrev"] .'&nbsp;<b>'. $game["hscore"] .'</b></div>';
      }
      $output .= "</td></tr>";
    }
    $output .= '</table></div>';
  } 
  else {
    drupal_set_title("$title - Week $weekarg");
    $query = "SELECT g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, g.gametime FROM {pickem_games} g where g.wid=%d ORDER BY g.wid, g.gametime, g.gid";
    $result = db_query($query, $weekarg);
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
    $output = '<div class="pickem"><form action="" method="post"><table>';
    $output .= "<tr><th>Visitor</th><th>Home Team</th><th>Game Time(EST)</th></tr>";
    foreach ($games as $wid => $weeksgames) {
      foreach ($weeksgames as $gid => $game) {
        $output .= '<tr>';
        $output .= '<td style="width:33%;font-weight:bold;"><input name="gid_'. $gid .'_'. $game['vid'] .'" type="text" size="3" value="'. $game['vscore'] .'" />'. $teams[$game["vid"]]["abbrev"] .'</td>';
        $output .= '<td style="width:33%;font-weight:bold;"><input name="gid_'. $gid .'_'. $game['hid'] .'" type="text" size="3" value="'. $game['hscore'] .'" />'. $teams[$game["hid"]]["abbrev"] .'</td>';
        $output .= '<td style="width:33%;">'. $game['gametime'] .'</td></tr>';
      }
    }
    $output .= '</table>';
    //if ($week[$weekarg]['week_past'] == 1) {
      $output .= '<input type="submit" value="savescores"/>';
    //}
    $output .= '</form></div>';
  }

  print theme('page', $output);
}



















