<?php
// $Id: pickem.inc,v 1.1.2.2 2008/09/19 14:35:38 jvandervort Exp $

/**
 * @file
 * includes little helpers functions.
*/

function get_weeks($weekarg = 0, $season_segment = 'regular') {
  if ($weekarg == 0) {
    if ( $season_segment == 'regular' ) {
      $query = "SELECT distinct pw.wid,pw.wk_name,pw.wk_firstgame,pw.wk_points from {pickem_weeks} pw where pw.playoffs=0 order by pw.wid, pw.wk_firstgame ";
    } 
    else if ( $season_segment == 'playoffs' ) {
      $query = "SELECT distinct pw.wid,pw.wk_name,pw.wk_firstgame,pw.wk_points from {pickem_weeks} pw where pw.playoffs=1 order by pw.wid, pw.wk_firstgame ";
    } 
    else {
      $query = "SELECT distinct pw.wid,pw.wk_name,pw.wk_firstgame,pw.wk_points from {pickem_weeks} pw order by pw.wid, pw.wk_firstgame ";
    }
    $result = db_query($query);
  } 
  else {
    $query = "SELECT distinct pw.wid,pw.wk_name,pw.wk_firstgame,pw.wk_points from {pickem_weeks} pw where pw.wid=%d order by pw.wk_firstgame";
    $result = db_query($query, $weekarg);
  }
  $weeks = array();
  while ($row = db_fetch_object($result)) {
    $week_past = 0;
    if (date('Y-m-d H:i:s', time()) > $row->wk_firstgame && $row->wk_firstgame <> 0) {
      $week_past = 1;
    }
    
    $weeks[$row->wid] = array("week_past" => $week_past);
    foreach ($row as $key => $value) {
      $weeks[$row->wid] = array_merge($weeks[$row->wid], array($key => $value));
    }
    
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
    $users[$row->uid] = array(
      "uid" => $row->uid,
      "name" => $row->name,
      "mail" => $row->mail,
    );
  }
  return $users;
}


/**
 * Custom wildcard loaders for use in hook_menu()
*/
function league_load($league_id) {
  if ( is_numeric($league_id) ) {
    $ret = get_leagues($league_id);
    return $ret[0];
  }
  else {
    return FALSE;
  }
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
    $leagues[] = $row;
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



function get_teams() {
  $query = "SELECT * from {pickem_teams} order by tm_abbrev";
  $result = db_query($query);
  $teams = array();
  while ($row = db_fetch_object($result)) {
    $teams[$row->tid] = $row;
  }
  return $teams;
}


function get_games($week_id="") {

  //drupal_set_message($week_id);

  if (is_numeric($week_id)) {
    $query = "SELECT g.* FROM {pickem_games} g inner join {pickem_weeks} pw on pw.wid=g.wid where g.wid=%d order by g.wid, g.gametime, g.gid";
    $result = db_query($query, $week_id);
  } 
  else {
    $query = "SELECT g.* FROM {pickem_games} g inner join {pickem_weeks} pw on pw.wid=g.wid order by g.wid, g.gametime, g.gid";
    $result = db_query($query);
    
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
  }
  else {
	  // two parts query so current week is always ontop, and passed weeks are at the bottom.
	  $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, pp.winnerpick_id, g.gametime from {pickem_games} g inner join {pickem_weeks} w on w.wid=g.wid left outer join {pickem_picks} pp on pp.gid = g.gid and pp.uid=%d and pp.lid=%d where w.wk_firstgame > now() order by g.wid, g.gametime, g.gid";
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
	  
	  $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, pp.winnerpick_id, g.gametime from {pickem_games} g inner join {pickem_weeks} w on w.wid=g.wid left outer join {pickem_picks} pp on pp.gid = g.gid and pp.uid=%d and pp.lid=%d where w.wk_firstgame < now() order by g.wid, g.gametime, g.gid";
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
    $query = 'select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.v_spread, g.h_id, g.h_spread, p.uid, p.winnerpick_id, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wk_firstgame<now() order by g.wid, g.gametime, g.gid';
    $result = db_query($query, $league_id);
  } 
  else {
    // all games for a certain week
    $query = 'select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.v_spread, g.h_id, g.h_spread, p.uid, p.winnerpick_id, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wid=%d order by g.wid, g.gametime, g.gid';
    $result = db_query($query, $league_id, $weekarg);
  }

  $totals = array();
  $users = get_users($league_id);

  foreach ($users as $u) {
    $totals[$u['uid']] = array(
      'total' => 0,
      'name' => $u['name']
    );
  }

  while ($row = db_fetch_object($result)) {
    //print_r($row);
    
    if (!isset ($totals[$row->uid][$row->wid])) {
      $totals[$row->uid][$row->wid] = 0;
    }

    if ( $league->scoring_type == 0 ) {
      // straight win-loss
      if ($row->h_score > $row->v_score && $row->winnerpick_id == $row->h_id) {
        $totals[$row->uid][$row->wid] += $row->wk_points;
        $totals[$row->uid]['total'] += $row->wk_points;
      } 
      else if ($row->v_score > $row->h_score && $row->winnerpick_id == $row->v_id) {
          $totals[$row->uid][$row->wid] += $row->wk_points;
          $totals[$row->uid]['total'] += $row->wk_points;
      } 
      else if ($row->v_score == $row->h_score && $row->h_score <> "") {
            $totals[$row->uid][$row->wid] += $row->wk_points;
            $totals[$row->uid]['total'] += $row->wk_points;
      }
    }
    else if ( $league->scoring_type == 1 ) {
      // use spread
      if ( ($row->h_score + $row->h_spread) > $row->v_score && $row->winnerpick_id == $row->h_id) {
        $totals[$row->uid][$row->wid] += $row->wk_points;
        $totals[$row->uid]['total'] += $row->wk_points;
      } 
      else if ( ($row->v_score + $row->v_spread) > $row->h_score && $row->winnerpick_id == $row->v_id) {
        $totals[$row->uid][$row->wid] += $row->wk_points;
        $totals[$row->uid]['total'] += $row->wk_points;
      } 
      else if ( ($row->v_score + $row->v_spread) == $row->h_score && $row->h_score <> "") {
        $totals[$row->uid][$row->wid] += $row->wk_points;
        $totals[$row->uid]['total'] += $row->wk_points;
      }
    }
  }
  uasort($totals, "sort_totals");
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
  if ($nextweek == TRUE) {
    // once you pass the firstgame start you are on the next week
    $row = db_fetch_object(db_query("SELECT pw.wid,pw.wk_firstgame, UNIX_TIMESTAMP(pw.wk_firstgame) - UNIX_TIMESTAMP(CURDATE()) as diff FROM {pickem_weeks} pw where wk_firstgame > now() order by wk_firstgame"));
  } 
  else {
    // ONLY once you pass the firstgame aer you on THIS week.
    $row = db_fetch_object(db_query("SELECT pw.wid,pw.wk_firstgame, UNIX_TIMESTAMP(pw.wk_firstgame) - UNIX_TIMESTAMP(CURDATE()) as diff FROM {pickem_weeks} pw where wk_firstgame < now() order by wk_firstgame DESC"));
  }
  $w = array();
  $w['wid'] = $row->wid;
  $w['days'] = ($row->diff) / 86400; // days: decimel number
  $w['firstgame'] = $row->wk_firstgame;
  
  $dow = date("l", strtotime($w['firstgame']));
  
  
  $fg = preg_split('/ /', $w['firstgame']);
  $w['firstgame_date'] = $fg[0];
  $w['firstgame_time'] = $fg[1];
  $w['firstgame_xml'] = $fg[0] .'T'. $fg[1] .'-04:00';
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
  $totals = get_user_totals($league_id, 0);
  $weeks = get_weeks(0, 'regular');
  $wk_summary = array();
  foreach ($weeks as $w => $w_data) {
    if ( $w_data['week_past']==1 ) {
      $max = 0;
      $min = 100;
      $tot = 0;
      $count = 0;
      foreach ($totals as $picker => $user_totals) {
        if ( isset($user_totals[$w]) ) {
          if ($user_totals[$w] > $max) {
            $max = $user_totals[$w];
          }
          if ( $user_totals[$w] < $min ) {
            $min = $user_totals[$w];
          }
          $tot += $user_totals[$w];
          $count += 1;
        }
      }
      $wk_summary[] = array(
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


function get_team_totals($league_id, $sort_by='win_loss') {
  // all games
  $query = 'select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, p.uid, p.winnerpick_id, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wk_firstgame<now() order by g.wid, g.gametime, g.gid';
  $result = db_query($query, $league_id);

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
    
    if ( $totals[$row->h_id]['total_picks'] != 0 ) {
      $totals[$row->h_id]['pickability'] = round($totals[$row->h_id]['correct_picks'] / $totals[$row->h_id]['total_picks'] * 100, 1);
      $totals[$row->h_id]['pickability_w'] = round($totals[$row->h_id]['win_picks'] / $totals[$row->h_id]['total_picks'] * 100, 1);
    }
    if ( $totals[$row->v_id]['total_picks'] != 0 ) {
      $totals[$row->v_id]['pickability'] = round($totals[$row->v_id]['correct_picks'] / $totals[$row->v_id]['total_picks'] * 100, 1);
      $totals[$row->v_id]['pickability_w'] = round($totals[$row->v_id]['win_picks'] / $totals[$row->v_id]['total_picks'] * 100, 1);
    }
    
  }
  uasort($totals, "sort_$sort_by");
  return $totals;
}
function sort_win_lose($a, $b) {
  if ($a['pickability'] > $b['pickability']) {
    return -1;
  } 
  else if ($a['pickability'] < $b['pickability']) {
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