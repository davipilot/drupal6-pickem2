<?php
// $Id: pickem.inc,v 1.1.2.2 2008/09/19 14:35:38 jvandervort Exp $

/**
 * @file
 * includes little helpers functions.
*/

/**
 * Custom wildcard loaders for use in hook_menu()
*/
function league_load($league_id) {
  $valid = TRUE;
  $league = FALSE;
  if ( is_numeric($league_id) ) {
    $ret = get_leagues($league_id);
    if ( count($ret) > 0 ) {
      $tmp = each($ret);
      $league = $tmp['value'];
    }
    else {
      $valid = FALSE;
    }
  }
  return $valid ? $league : FALSE;
}
function team_load($team_id) {
  $valid = TRUE;
  $team = FALSE;
  if ( is_numeric($team_id) ) {
    $ret = get_teams(array('team_id' => $team_id));
    if ( count($ret) > 0 ) {
      $tmp = each($ret);
      $team = $tmp['value'];
    }
    else {
      $valid = FALSE;
    }
  }
  return $valid ? $team : FALSE;
}
function game_load($game_id) {
  $valid = TRUE;
  $game = FALSE;
  if ( is_numeric($game_id) ) {
    $ret = get_games(array('game_id' => $game_id));
    if ( count($ret) > 0 ) {
      $tmp = each($ret);
      $game = $tmp['value'];
    }
    else {
      $valid = FALSE;
    }
  }
  return $valid ? $game : FALSE;
}
function week_load($week_id) {
  $valid = TRUE;
  $week = FALSE;
  if ( is_numeric($week_id) ) {
    $ret =  get_weeks(array('week_id' => $week_id));
    if ( count($ret) > 0 ) {
      $tmp = each($ret);
      $week = $tmp['value'];
    }
    else {
      $valid = FALSE;
    }
  }
  return $valid ? $week : FALSE;
}
function gameset_load($gameset_id) {
  $valid = TRUE;
  $gameset = FALSE;
  if ( is_numeric($gameset_id) ) {
    $ret = get_gamesets(array('gameset_id' => $gameset_id));
    if ( count($ret) > 0 ) {
      $tmp = each($ret);
      $gameset = $tmp['value'];
    }
    else {
      $valid = FALSE;
    }
  }
  return $valid ? $gameset : FALSE;
}


function bad_league_week($league, $week) {
  $query = "SELECT lid, wid from {pickem_leagues} l inner join {pickem_weeks} w on w.gsid=l.gsid WHERE lid=%d and wid=%d";
  $result = db_query($query, $league->lid, $week->wid);
  $row = db_fetch_object($result);
  if ( isset($row->lid) ) {
    return 0;
  }
  else {
    return 1;
  }
}

function get_weeks( $filter = array('league_id' => NULL, 'gameset_id' => NULL, 'week_id' => NULL, 'playoffs' => NULL) ) {

  $sql_where_list = array();
  if ( !is_null($filter['gameset_id'] ) ) {
    $sql_where_list[] = 'gsid='. $filter['gameset_id'];
  }
  if ( !is_null($filter['week_id'] ) ) {
    $sql_where_list[] = 'wid='. $filter['week_id'];
  }
  if ( !is_null($filter['playoffs'] ) ) {
    $sql_where_list[] = 'playoffs='. $filter['playoffs'];
  }
  if ( !is_null($filter['league_id'] ) ) {
    $sql_where_list[] = 'l.lid='. $filter['league_id'];
  }

  $sql_where = '';
  if ( count($sql_where_list) > 0 ) {
    $sql_where = ' WHERE '. implode(' and ', $sql_where_list);
  }

  // different queries based on filter
  if ( is_null($filter['league_id'] ) ) {
    $query = "SELECT * from {pickem_weeks} ". $sql_where ." ORDER BY wk_firstgame, wk_number";
  }
  else {
    $query = "SELECT w.* from {pickem_weeks} w inner join {pickem_leagues} l on l.gsid=w.gsid ". $sql_where ." ORDER BY wk_firstgame, wk_number";
  }

  $result = db_query($query);

  $weeks = array();
  while ($row = db_fetch_object($result)) {
    $week_past = 0;
    if (date('Y-m-d H:i:s', time()) > $row->wk_firstgame && $row->wk_firstgame <> 0) {
      $week_past = 1;
    }
    $row->week_past = $week_past;
    $weeks[$row->wid] = $row;
  }

  return $weeks;
}


function get_users($league=NULL, $not_in_league=0) {

  if ( !isset($league) ) {
    $query = "SELECT u.* from {users} u where u.uid<>0 order by name";
    $result = db_query($query);
  }
  else {
    if ( $not_in_league == 0 ) {
      //drupal_set_message("All users IN league " . $league->lid);
      $query = "SELECT u.* from {users} u inner join {pickem_users} pu on pu.uid=u.uid where u.uid<>0 and pu.lid=%d order by name";
    }
    else {
      $query = "SELECT * from {users} where uid not in (select uid from {pickem_users} where lid=%d) and uid<>0 order by name";
    }
    $result = db_query($query, $league->lid);
  }
  $users = array();
  while ($row = db_fetch_object($result)) {
    $users[$row->uid] = $row;
  }
  return $users;
}

function get_leagues($league_id=0) {
  if ( $league_id == 0 ) {
    //all leagues
    $query = "SELECT * from {pickem_leagues} order by name";
    $result = db_query($query);
  }
  else {
    //one league
    $query = "SELECT * from {pickem_leagues} where lid=%d order by name";
    $result = db_query($query, $league_id);
  }
  $leagues = array();
  while ($row = db_fetch_object($result)) {
    $leagues[$row->lid] = $row;
  }

  return $leagues;
}
function get_my_leagues($gameset_id=NULL) {
  global $user;
  if ( is_null($gameset_id) ) {
    $query = "SELECT l.* from {pickem_leagues} l inner join {pickem_users} u on u.lid=l.lid where u.uid=%d";
    $result = db_query($query, $user->uid);
  }
  else {
    $query = "SELECT l.* from {pickem_leagues} l inner join {pickem_users} u on u.lid=l.lid where u.uid=%d and l.gsid=%d";
    $result = db_query($query, $user->uid, $gameset_id);
  }
  $leagues = array();
  while ($row = db_fetch_object($result)) {
    $leagues[] = $row;
  }
  return $leagues;
}
function is_my_league($league) {
  global $user;
  $query = "SELECT l.* from {pickem_leagues} l inner join {pickem_users} pu on pu.lid=l.lid where pu.lid=%d and pu.uid=%d";
  $row = db_fetch_object(db_query($query, $league->lid, $user->uid));
  if ( $row ) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}


function get_teams($filter = array('team_id' => NULL, 'gameset_id' => NULL)) {
  if ( !is_null($filter['gameset_id']) ) {
    $query = "SELECT * from {pickem_teams} where gsid=%d order by tm_abbrev, tm_name";
    $result = db_query($query, $filter['gameset_id']);
  }
  else if ( !is_null($filter['team_id']) ) {
    $query = "SELECT * from {pickem_teams} where tid=%d order by tm_abbrev, tm_name";
    $result = db_query($query, $filter['team_id']);
  }
  else {
    $query = "SELECT * from {pickem_teams} order by tm_abbrev, tm_name";
    $result = db_query($query, $filter['team_id']);
  }
  $teams = array();
  $module_path = drupal_get_path('module', 'pickem');
  while ($row = db_fetch_object($result)) {
    $row->short_name = ($row->tm_abbrev <> '') ? $row->tm_abbrev : $row->tm_name;
    $row->long_name = $row->tm_city .' '. $row->tm_name;
    if ( $row->tm_abbrev <> '' ) {
      $row->long_name .=  ' ('. $row->tm_abbrev .')';
    }
    if ($row->logo_small != '') {
      $row->small_logo_path = "/$module_path/logos/".$row->logo_small;
      $row->small_logo_img = '<img src="' . $row->small_logo_path . '" />';
    } else {
      $row->small_logo_path = '';
      $row->small_logo_img = '';
    }
    $teams[$row->tid] = $row;
  }
  return $teams;
}
function get_gamesets($filter = array('gameset_id' => NULL, 'gameset_name' => NULL)) {
  if ( !is_null($filter['gameset_id']) ) {
    $query = "SELECT * from {pickem_gamesets} where gsid=%d order by gs_name";
    $result = db_query($query, $filter['gameset_id']);
  }
  elseif ( !is_null($filter['gameset_name']) ) {
    $query = "SELECT * from {pickem_gamesets} where gs_name='%s' order by gs_name";
    $result = db_query($query, $filter['gameset_name']);
  }
  else {
    //all gamesets
    $query = "SELECT * from {pickem_gamesets} order by gs_name";
    $result = db_query($query);
  }
  $gamesets = array();
  while ($row = db_fetch_object($result)) {
    $gamesets[$row->gsid] = $row;
  }

  return $gamesets;
}

function get_games($filter=array('game_id' => NULL, 'week_id' => NULL, 'gameset_id' => NULL, 'team_id' => NULL, 'league_id' => NULL), $options=array('by_week' => 0) ) {

  $sql_where_list = array();
  if ( !is_null($filter['game_id'] ) ) {
    $sql_where_list[] = 'g.gid='. $filter['game_id'];
  }
  if ( !is_null($filter['week_id'] ) ) {
    $sql_where_list[] = 'g.wid='. $filter['week_id'];
  }
  if ( !is_null($filter['gameset_id'] ) ) {
    $sql_where_list[] = 'pw.gsid='. $filter['gameset_id'];
  }
  if ( !is_null($filter['league_id'] ) ) {
    $sql_where_list[] = 'l.lid='. $filter['league_id'];
  }
  if ( !is_null($filter['team_id'] ) ) {
    $sql_where_list[] = '(g.v_id='. $filter['team_id'] .' or g.h_id='. $filter['team_id'] .')';
  }

  $sql_where = '';
  if ( count($sql_where_list) > 0 ) {
    $sql_where = ' WHERE '. implode(' and ', $sql_where_list);
  }

  $games = array();
  $mysqltime = date("Y-m-d H:i:s");

  if ( is_null($filter['league_id'] ) ) {
    $query = "SELECT g.*, case when '$mysqltime' > pw.wk_firstgame then 1 else 0 end as 'week_past', pw.wk_number FROM {pickem_games} g INNER JOIN {pickem_weeks} pw ON pw.wid=g.wid  ". $sql_where ." ORDER BY week_past, g.gametime, g.gid";
  }
  else {
    $query = "SELECT g.*, case when '$mysqltime' > pw.wk_firstgame then 1 else 0 end as 'week_past', pw.wk_number FROM {pickem_games} g INNER JOIN {pickem_weeks} pw ON pw.wid=g.wid inner join {pickem_leagues} l on l.gsid=pw.gsid ". $sql_where ." ORDER BY week_past, g.gametime, g.gid";
  }

  $result = db_query($query);
  while ($row = db_fetch_object($result)) {
    if ( $options['by_week'] == 1 ) {
      $games[$row->wid][$row->gid] = $row;
    }
    else {
      $games[$row->gid] = $row;
    }
  }
  return $games;
}

function get_picks($filter = array('league_id' => NULL, 'user_id' => NULL, 'week_id' => NULL) ) {

  if ( !is_null($filter['league_id'] ) ) {
    $sql_where_list[] = 'l.lid='. $filter['league_id'];
  }
  if ( !is_null($filter['user_id'] ) ) {
    $sql_where_list[] = 'u.uid='. $filter['user_id'];
  }
  if ( !is_null($filter['week_id'] ) ) {
    $sql_where_list[] = 'w.wid='. $filter['week_id'];
  }

  $sql_where = '';
  if ( count($sql_where_list) > 0 ) {
    $sql_where = ' WHERE '. implode(' and ', $sql_where_list);
  }

  $query = 'SELECT g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, p.uid, p.winnerpick_id, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid inner join {pickem_leagues} l on l.gsid=w.gsid and l.lid=u.lid '. $sql_where .' ORDER BY g.wid, g.gametime, g.gid';
  $result = db_query($query);

  $picks = array();
  while ($row = db_fetch_object($result)) {
    $picks[$row->gid][$row->uid] = $row;
  }

  return $picks;
}
function set_pick($game_id, $user_id, $league_id, $winnerpick_id) {

  $query = 'select count(*) as pickcount from {pickem_picks} where gid=%d and uid=%d and lid=%d';
  $result = db_query($query, $game_id, $user_id, $league_id);
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
    $result = db_query($query, $game_id, $user_id, $league_id, $winnerpick_id, date('Y-m-d H:i:s', time()));
    if ( $result == FALSE ) {
      drupal_set_message('set_pick failed', 'error');
      watchdog('pickem', 'set_pick failed');
    }
  }
  elseif ($type == "update") {
    $query = "update {pickem_picks} set winnerpick_id=%d,picktime='%s' where gid=%d and uid=%d and lid=%d";
    $result = db_query($query, $winnerpick_id, date('Y-m-d H:i:s', time()), $game_id, $user_id, $league_id);
    if ( $result == FALSE ) {
      drupal_set_message('set_pick failed', 'error');
      watchdog('pickem', 'set_pick failed');
    }
  }
}


function get_user_totals($league=NULL, $week_id=0) {
  $totals = array();
  $users = get_users($league);
  foreach ($users as $u) {
    $totals[$u->uid] = array(
      'total' => 0,
      'name' => $u->name,
      'best_week_id' => -1,
      'worst_week_id' => -1,
      'ranking' => 0,
      'weeks' => array()
    );
  }

  if ($week_id == 0) {
    // all games
    $mysqltime = date("Y-m-d H:i:s");
    $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.v_spread, g.h_id, g.h_spread, p.uid, p.winnerpick_id, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wk_firstgame < '". $mysqltime ."' order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $league->lid);
  }
  else {
    // all games for a certain week
    $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.v_spread, g.h_id, g.h_spread, p.uid, p.winnerpick_id, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wid=%d order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $league->lid, $week_id);
  }

  while ($row = db_fetch_object($result)) {
    //print_r($row);

    if (!isset ($totals[$row->uid]['weeks'][$row->wid])) {
      $totals[$row->uid]['weeks'][$row->wid] = 0;
    }

    if ( $league->scoring_type == 0 ) {
      // straight win-loss
      if ($row->h_score > $row->v_score && $row->winnerpick_id == $row->h_id) {
        $totals[$row->uid]['weeks'][$row->wid] += $row->wk_points;
        $totals[$row->uid]['total'] += $row->wk_points;
      }
      else if ($row->v_score > $row->h_score && $row->winnerpick_id == $row->v_id) {
          $totals[$row->uid]['weeks'][$row->wid] += $row->wk_points;
          $totals[$row->uid]['total'] += $row->wk_points;
      }
      else if ($row->v_score == $row->h_score && $row->h_score <> "") {
            $totals[$row->uid]['weeks'][$row->wid] += $row->wk_points;
            $totals[$row->uid]['total'] += $row->wk_points;
      }
    }
    else if ( $league->scoring_type == 1 ) {
      // use spread
      if ( ($row->h_score + $row->h_spread) > $row->v_score && $row->winnerpick_id == $row->h_id) {
        $totals[$row->uid]['weeks'][$row->wid] += $row->wk_points;
        $totals[$row->uid]['total'] += $row->wk_points;
      }
      else if ( ($row->v_score + $row->v_spread) > $row->h_score && $row->winnerpick_id == $row->v_id) {
        $totals[$row->uid]['weeks'][$row->wid] += $row->wk_points;
        $totals[$row->uid]['total'] += $row->wk_points;
      }
      else if ( ($row->v_score + $row->v_spread) == $row->h_score && $row->h_score <> "") {
        $totals[$row->uid]['weeks'][$row->wid] += $row->wk_points;
        $totals[$row->uid]['total'] += $row->wk_points;
      }
    }
  }

  // best and worst weeks
  foreach ($totals as $uid => $u_object) {
    foreach ($u_object['weeks'] as $wid => $score) {
      if ( $totals[$uid]['best_week_id'] == -1 ) {
        $totals[$uid]['best_week_id'] = $wid;
        $totals[$uid]['worst_week_id'] = $wid;
      }
      if ( $score > $totals[$uid]['weeks'][$totals[$uid]['best_week_id']] ) {
        $totals[$uid]['best_week_id'] = $wid;
      }
      if ( $score < $totals[$uid]['weeks'][$totals[$uid]['worst_week_id']] ) {
        $totals[$uid]['worst_week_id'] = $wid;
      }
    }
  }

  // total current ranking
  uasort($totals, "sort_totals");
  $ordinal = 0;
  $last_ordinal = 0;
  $high_score = 0;
  $last_score = 100000;
  foreach ($totals as $uid => $u_object) {
    $ordinal++;
    if ($u_object['total'] < $last_score) {
      $last_ordinal = $ordinal;
      $last_score = $u_object['total'];
      if ( $ordinal == 1 ) {
        $high_score = $last_score;
      }
    }
    $totals[$uid]['ranking'] = $last_ordinal;
    $totals[$uid]['pts_back'] = $high_score - $last_score;
  }

  return $totals;
}
function sort_totals($a, $b) {
  if ($a['total'] > $b['total']) {
    return -1;
  }
  else if ($a['total'] < $b['total']) {
      return 1;
  }
  else {
    if ($a['name'] < $b['name']) {
      return -1;
    }
    else if ($a['name'] > $b['name']) {
        return 1;
    }
    else {
      return 0;
    }
  }
}

function get_current_week($league, $nextweek = TRUE) {
  $mysqltime = date("Y-m-d H:i:s");
  if ($nextweek == TRUE) {
    // once you pass the firstgame start you are on the next week
    $row = db_fetch_object(db_query("SELECT *, UNIX_TIMESTAMP(wk_firstgame) - UNIX_TIMESTAMP('". $mysqltime ."') as diff FROM {pickem_weeks} w inner join {pickem_leagues} l on l.gsid=w.gsid where l.lid=%d and wk_firstgame > '". $mysqltime ."' order by wk_firstgame", $league->lid));
  }
  else {
    // ONLY once you pass the firstgame are you on THIS week.
    $row = db_fetch_object(db_query("SELECT *, UNIX_TIMESTAMP(wk_firstgame) - UNIX_TIMESTAMP('". $mysqltime ."') as diff FROM {pickem_weeks} w inner join {pickem_leagues} l on l.gsid=w.gsid where l.lid=%d and wk_firstgame <= '". $mysqltime ."' order by wk_firstgame DESC", $league->lid));
  }
  $row->days = ($row->diff) / 86400; // days: decimel number
  $row->firstgame = $row->wk_firstgame;
  $row->firstgame_dow = date("l", strtotime($row->wk_firstgame));
	$tz_offset = date("O", strtotime($row->wk_firstgame));
	$tz_offset = substr($tz_offset, 0, 3) . ':' . substr($tz_offset, 3);
	
  $fg = preg_split('/ /', $row->wk_firstgame);
  $row->firstgame_date = $fg[0];
  $row->firstgame_time = $fg[1];
  $row->firstgame_xml = $fg[0] .'T'. $fg[1] .$tz_offset;

  return $row;
}


function get_totals_by_week($league) {
  $totals = get_user_totals($league);
  $weeks = get_weeks(array('gameset_id' => $league->gsid));
  $wk_summary = array();
  foreach ($weeks as $wid => $week) {
    if ( $week->week_past==1 ) {
      $max = 0;
      $min = 100;
      $tot = 0;
      $count = 0;
      foreach ($totals as $uid => $u) {
        if ( isset($u['weeks'][$wid]) ) {
          if ($u['weeks'][$wid] > $max) {
            $max = $u['weeks'][$wid];
          }
          if ( $u['weeks'][$wid] < $min ) {
            $min = $u['weeks'][$wid];
          }
          $tot += $u['weeks'][$wid];
          $count += 1;
        }
      }

      if ( $max == $min ) {
        $max = '';
        $min = '';
      }

      $wk_summary[$wid] = array(
        "name" => $week->wk_number,
        "max" => $max,
        "min" => $min,
        "count" => $count,
        "total" => $tot,
      );
    }
  }
  return $wk_summary;
}


function get_team_totals($league, $sort_by='win_lose', $week_id=0) {

  $totals = array();
  $teams = get_teams(array('gameset_id' => $league->gsid));

  foreach ($teams as $tid => $team) {
    $totals[$tid] = array(
      'total_picks' => 0,
      'win_picks' => 0,
      'correct_picks' => 0,
      'pickability' => 'NULL',
      'pickability_w' => 'NULL',
      'wins' => 0,
      'abbrev' => $team->tm_abbrev,
      'times_picked' => 0,
      'picked_perc' => NULL,
    );
  }

  $mysqltime = date("Y-m-d H:i:s");
  if ( $week_id != 0 ) {
    $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, p.uid, p.winnerpick_id, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wk_firstgame < '". $mysqltime ."' and w.wid=%d order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $league->lid, $week_id);
  }
  else {
    $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, p.uid, p.winnerpick_id, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wk_firstgame < '". $mysqltime ."' order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $league->lid);
  }
  while ($row = db_fetch_object($result)) {
    //print_r($row);

    // Home Win, Correct
    if ($row->h_score > $row->v_score && $row->winnerpick_id == $row->h_id) {
      $totals[$row->v_id]['correct_picks'] += 1;
      $totals[$row->h_id]['correct_picks'] += 1;
      $totals[$row->h_id]['win_picks'] += 1;
      $totals[$row->h_id]['wins'] += 1;
    }
    // Visitor Win, Correct
    else if ($row->v_score > $row->h_score && $row->winnerpick_id == $row->v_id) {
      $totals[$row->v_id]['correct_picks'] += 1;
      $totals[$row->v_id]['win_picks'] += 1;
      $totals[$row->h_id]['correct_picks'] += 1;
      $totals[$row->v_id]['wins'] += 1;
    }
    // Tie
    else if ($row->v_score == $row->h_score && $row->h_score <> "") {
      $totals[$row->v_id]['correct_picks'] += 1;
      $totals[$row->h_id]['correct_picks'] += 1;
    }

    $totals[$row->v_id]['total_picks'] += 1;
    $totals[$row->h_id]['total_picks'] += 1;

    if ( $row->winnerpick_id == $row->h_id ) {
      $totals[$row->h_id]['times_picked'] += 1;
    }

    if ( $row->winnerpick_id == $row->v_id ) {
      $totals[$row->v_id]['times_picked'] += 1;
    }

    if ( $totals[$row->h_id]['total_picks'] != 0 ) {
      $totals[$row->h_id]['pickability'] = round($totals[$row->h_id]['correct_picks'] / $totals[$row->h_id]['total_picks'] * 100, 1);
      $totals[$row->h_id]['pickability_w'] = round($totals[$row->h_id]['win_picks'] / $totals[$row->h_id]['total_picks'] * 100, 1);
      $totals[$row->h_id]['picked_perc'] = round($totals[$row->h_id]['times_picked'] / $totals[$row->h_id]['total_picks'] * 100, 0);
    }
    if ( $totals[$row->v_id]['total_picks'] != 0 ) {
      $totals[$row->v_id]['pickability'] = round($totals[$row->v_id]['correct_picks'] / $totals[$row->v_id]['total_picks'] * 100, 1);
      $totals[$row->v_id]['pickability_w'] = round($totals[$row->v_id]['win_picks'] / $totals[$row->v_id]['total_picks'] * 100, 1);
      $totals[$row->v_id]['picked_perc'] = round($totals[$row->v_id]['times_picked'] / $totals[$row->v_id]['total_picks'] * 100, 0);
    }


  }
  uasort($totals, "sort_$sort_by");
  return $totals;
}
function sort_win_lose($a, $b) {
  if ($a['pickability'] == NULL || $b['pickability'] == NULL) {
    return 0;
  }
  elseif ($a['pickability'] > $b['pickability']) {
    return -1;
  }
  elseif ($a['pickability'] < $b['pickability']) {
      return 1;
  }
  else {
    if ($a['abbrev'] < $b['abbrev']) {
      return -1;
    }
    else if ($a['abbrev'] > $b['abbrev']) {
        return 1;
    }
    else {
      return 0;
    }
  }
}
function sort_to_win($a, $b) {
  if ($a['pickability_w'] > $b['pickability_w']) {
    return -1;
  }
  else if ($a['pickability_w'] < $b['pickability_w']) {
      return 1;
  }
  else {
    if ($a['abbrev'] < $b['abbrev']) {
      return -1;
    }
    else if ($a['abbrev'] > $b['abbrev']) {
        return 1;
    }
    else {
      return 0;
    }
  }
}