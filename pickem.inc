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
    $ret = get_teams($team_id);
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
    $ret = get_games(NULL, NULL, $game_id);
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
    $ret = get_weeks($week_id);
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


function get_weeks($weekarg = 0, $season_segment = 'regular') {
  if ($weekarg == 0) {
    if ( $season_segment == 'regular' ) {
      $query = "SELECT * from {pickem_weeks} pw where pw.playoffs=0 order by pw.wid, pw.wk_firstgame ";
    }
    else if ( $season_segment == 'playoffs' ) {
      $query = "SELECT * from {pickem_weeks} pw where pw.playoffs=1 order by pw.wid, pw.wk_firstgame ";
    }
    else {
      $query = "SELECT * from {pickem_weeks} pw order by pw.wid, pw.wk_firstgame ";
    }
    $result = db_query($query);
  }
  else {
    $query = "SELECT * from {pickem_weeks} pw where pw.wid=%d order by pw.wk_firstgame";
    $result = db_query($query, $weekarg);
  }
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


function get_users($league_id=0, $not_in_league=0) {
  if ( $league_id==0 ) {
    //drupal_set_message("All Users In All Leagues " . $league_id);
    $query = "SELECT u.* from {users} u where u.uid<>0 order by name";
    $result = db_query($query);
  }
  else {
    if ( $not_in_league == 0 ) {
      //drupal_set_message("All Users In League " . $league_id);
      $query = "SELECT u.* from {users} u inner join {pickem_users} pu on pu.uid=u.uid where u.uid<>0 and pu.lid=%d order by name";
    }
    else {
      $query = "SELECT * from {users} where uid not in (select uid from {pickem_users} where lid=%d) and uid<>0 order by name";
    }
    $result = db_query($query, $league_id);
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
function get_my_leagues() {
  global $user;
  $query = "SELECT l.* from {pickem_leagues} l inner join {pickem_users} pu on pu.lid=l.lid where pu.uid=%d";
  $result = db_query($query, $user->uid);
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


function get_teams($team_id=0) {
  if ( $team_id == 0 ) {
    //all teams
    $query = "SELECT * from {pickem_teams} order by tm_abbrev, tm_name";
    $result = db_query($query);
  }
  else {
    //one team
    $query = "SELECT * from {pickem_teams} where tid=%d order by tm_abbrev, tm_name";
    $result = db_query($query, $team_id);
  }
  $teams = array();
  while ($row = db_fetch_object($result)) {
    $row->short_name = ($row->tm_abbrev <> '') ? $row->tm_abbrev : $row->tm_name;
    $row->long_name = $row->tm_city . ' ' . $row->tm_name;
    if ( $row->tm_abbrev <> '' ) {
    	$row->long_name .=  ' (' . $row->tm_abbrev . ')';
    }
    $teams[$row->tid] = $row;
  }
  return $teams;
}


function get_games($week_id=NULL, $team_id=NULL, $game_id=NULL) {

  //drupal_set_message($week_id);
  if ( ! is_null($game_id ) ) {
    $query = "SELECT g.* FROM {pickem_games} g inner join {pickem_weeks} pw on pw.wid=g.wid where g.gid=%d order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $game_id);
  }
  else if ( is_null($week_id) && is_null($team_id) ) {
    $query = "SELECT g.* FROM {pickem_games} g inner join {pickem_weeks} pw on pw.wid=g.wid order by g.wid, g.gametime, g.gid";
    $result = db_query($query);
  }
  else if (is_numeric($week_id) && is_null($team_id) ) {
    $query = "SELECT g.* FROM {pickem_games} g inner join {pickem_weeks} pw on pw.wid=g.wid where g.wid=%d order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $week_id);
  }
  else if (is_numeric($week_id) && is_numeric($team_id) ) {
    $query = "SELECT g.* FROM {pickem_games} g inner join {pickem_weeks} pw on pw.wid=g.wid where g.wid=%d and (g.v_id=%d or g.h_id=%d) order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $week_id, $team_id, $team_id);
  }
  else if (is_null($week_id) && is_numeric($team_id) ) {
    $query = "SELECT g.* FROM {pickem_games} g inner join {pickem_weeks} pw on pw.wid=g.wid where (g.v_id=%d or g.h_id=%d) order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $team_id, $team_id);
  }

  $games = array();
  while ($row = db_fetch_object($result)) {
    $games[$row->gid] = $row;
  }
  return $games;
}


function get_games_split($league_id = 0, $user_id = 0) {
  //
  $games = array();
  if ( $league_id == 0 && $user_id == 0 ) {
    $mysqltime = date("Y-m-d H:i:s");
    $query = "SELECT g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, g.gametime FROM {pickem_games} g inner join {pickem_weeks} w on w.wid=g.wid where '". $mysqltime ."' < adddate(w.wk_firstgame, INTERVAL 6 day) ORDER BY g.wid, g.gametime, g.gid";
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

    $query = "SELECT g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, g.gametime FROM {pickem_games} g inner join {pickem_weeks} w on w.wid=g.wid where '". $mysqltime ."' > adddate(w.wk_firstgame, INTERVAL 6 day) ORDER BY g.wid, g.gametime, g.gid";
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
  }
  else {
    // two parts query so current week is always ontop, and passed weeks are at the bottom.
    $mysqltime = date("Y-m-d H:i:s");
    $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, pp.winnerpick_id, g.gametime from {pickem_games} g inner join {pickem_weeks} w on w.wid=g.wid left outer join {pickem_picks} pp on pp.gid = g.gid and pp.uid=%d and pp.lid=%d where w.wk_firstgame > '". $mysqltime ."' order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $user_id, $league_id);
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

    $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, pp.winnerpick_id, g.gametime from {pickem_games} g inner join {pickem_weeks} w on w.wid=g.wid left outer join {pickem_picks} pp on pp.gid = g.gid and pp.uid=%d and pp.lid=%d where w.wk_firstgame < '". $mysqltime ."' order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $user_id, $league_id);
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
  }
  return $games;
}



function get_game_picks($league_id=0, $weekarg=0) {
  if ($weekarg != 0) {

    $query = 'select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, p.uid, p.winnerpick_id, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wid=%d order by g.wid, g.gametime, g.gid';
    $result = db_query($query, $league_id, $weekarg);

    $games = array();
    while ($row = db_fetch_object($result)) {
      $games[$row->gid][$row->uid] = $row->winnerpick_id;
    }
    return $games;
  }
}

function get_user_totals($league_id, $weekarg=0) {
  $league = league_load($league_id);
  if ($weekarg == 0) {
    // all games
    $mysqltime = date("Y-m-d H:i:s");
    $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.v_spread, g.h_id, g.h_spread, p.uid, p.winnerpick_id, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wk_firstgame < '". $mysqltime ."' order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $league_id);
  }
  else {
    // all games for a certain week
    $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.v_spread, g.h_id, g.h_spread, p.uid, p.winnerpick_id, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wid=%d order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $league_id, $weekarg);
  }

  $totals = array();
  $users = get_users($league_id);

  foreach ($users as $u) {
    $totals[$u->uid] = array(
      'total' => 0,
      'name' => $u->name,
      'best_week_num' => -1,
      'worst_week_num' => -1,
      'ranking' => 0,
      'weeks' => array()
    );
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
      if ( $totals[$uid]['best_week_num'] == -1 ) {
        $totals[$uid]['best_week_num'] = $wid;
        $totals[$uid]['worst_week_num'] = $wid;
      }
      if ( $score > $totals[$uid]['weeks'][$totals[$uid]['best_week_num']] ) {
        $totals[$uid]['best_week_num'] = $wid;
      }
      if ( $score < $totals[$uid]['weeks'][$totals[$uid]['worst_week_num']] ) {
        $totals[$uid]['worst_week_num'] = $wid;
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

function get_current_week($nextweek = TRUE) {
  $mysqltime = date("Y-m-d H:i:s");
  if ($nextweek == TRUE) {
    // once you pass the firstgame start you are on the next week
    $row = db_fetch_object(db_query("SELECT pw.wid,pw.wk_firstgame, UNIX_TIMESTAMP(pw.wk_firstgame) - UNIX_TIMESTAMP('". $mysqltime ."') as diff FROM {pickem_weeks} pw where wk_firstgame > '". $mysqltime ."' order by wk_firstgame"));
  }
  else {
    // ONLY once you pass the firstgame are you on THIS week.
    $row = db_fetch_object(db_query("SELECT pw.wid,pw.wk_firstgame, UNIX_TIMESTAMP(pw.wk_firstgame) - UNIX_TIMESTAMP('". $mysqltime ."') as diff FROM {pickem_weeks} pw where wk_firstgame <= '". $mysqltime ."' order by wk_firstgame DESC"));
  }
  $w = array();
  $w['wid'] = $row->wid;
  $w['days'] = ($row->diff) / 86400; // days: decimel number
  $w['firstgame'] = $row->wk_firstgame;

  $dow = date("l", strtotime($w['firstgame']));


  $fg = preg_split('/ /', $w['firstgame']);
  $w['firstgame_date'] = $fg[0];
  $w['firstgame_time'] = $fg[1];
  $w['firstgame_xml'] = $fg[0] .'T'. $fg[1] .'-05:00';
  $w['dow'] = $dow;

  return $w;
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
  }
  elseif ($type == "update") {
    $query = "update {pickem_picks} set winnerpick_id=%d,picktime='%s' where gid=%d and uid=%d and lid=%d";
    $result = db_query($query, $winnerpick_id, date('Y-m-d H:i:s', time()), $game_id, $user_id, $league_id);
  }
}



function get_totals_by_week($league_id) {
  $totals = get_user_totals($league_id);
  $weeks = get_weeks(0, 'all');
  $wk_summary = array();
  foreach ($weeks as $w => $w_data) {
    if ( $w_data->week_past==1 ) {
      $max = 0;
      $min = 100;
      $tot = 0;
      $count = 0;
      foreach ($totals as $uid => $u) {
        if ( isset($u['weeks'][$w]) ) {
          if ($u['weeks'][$w] > $max) {
            $max = $u['weeks'][$w];
          }
          if ( $u['weeks'][$w] < $min ) {
            $min = $u['weeks'][$w];
          }
          $tot += $u['weeks'][$w];
          $count += 1;
        }
      }

      if ( $max == $min ) {
        $max = '';
        $min = '';
      }

      $wk_summary[$w] = array(
        "name" => $w,
        "max" => $max,
        "min" => $min,
        "count" => $count,
        "total" => $tot,
      );
    }
  }
  return $wk_summary;
}


function get_team_totals($league_id, $sort_by='win_lose', $week_id="") {
  // all games
  $mysqltime = date("Y-m-d H:i:s");
  if ( is_numeric($week_id) ) {
    $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, p.uid, p.winnerpick_id, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wk_firstgame < '". $mysqltime ."' and w.wid=%d order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $league_id, $week_id);
  }
  else {
    $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, p.uid, p.winnerpick_id, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wk_firstgame < '". $mysqltime ."' order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $league_id);
  }

  $totals = array();
  $teams = get_teams();

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