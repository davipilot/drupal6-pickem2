<?php
// $Id: pickem.inc,v 1.1.2.2 2008/09/19 14:35:38 jvandervort Exp $

/**
 * @file
 * includes little helpers functions.
*/

/**
 * Fetch a league object.
 * Also acts as a wildcard loader for hook_menu().
 *
 * @param $league_id
 *   A numeric id of the league to load.
 * @return
 *   A fully-loaded $league object upon successful league load or FALSE if league
 *   cannot be loaded.
 */
function league_load($league_id) {
  global $_PICKEM_SCORING_ENGINES;
  $valid = TRUE;
  $league = FALSE;
  if ( is_numeric($league_id) ) {
    $ret = get_leagues($league_id);
    if ( count($ret) > 0 ) {
      $tmp = each($ret);
      $league = $tmp['value'];
      $league->scoring_engine = $_PICKEM_SCORING_ENGINES[$league->scoring_type];
    }
    else {
      $valid = FALSE;
    }
  }
  return $valid ? $league : FALSE;
}

/**
 * Fetch a team object.
 * Also acts as a wildcard loader for hook_menu().
 *
 * @param $team_id
 *   A numeric id of the team to load.
 * @return
 *   A fully-loaded $team object upon successful team load or FALSE if team
 *   cannot be loaded.
 */
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

/**
 * Fetch a game object.
 * Also acts as a wildcard loader for hook_menu().
 *
 * @param $game_id
 *   A numeric id of the game to load.
 * @return
 *   A fully-loaded $game object upon successful game load or FALSE if game
 *   cannot be loaded.
 */
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

/**
 * Fetch a week object.
 * Also acts as a wildcard loader for hook_menu().
 *
 * @param $week_id
 *   A numeric id of the week to load.
 * @return
 *   A fully-loaded $week object upon successful week load or FALSE if week
 *   cannot be loaded.
 */
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

/**
 * Fetch a gameset object.
 * Also acts as a wildcard loader for hook_menu().
 *
 * @param $gameset_id
 *   A numeric id of the gameset to load.
 * @return
 *   A fully-loaded $gameset object upon successful gameset load or FALSE if gameset
 *   cannot be loaded.
 */
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

/**
 * Perform a check if the league has a particular week in its gameset.
 *
 * @param $league
 *   A league object.
 * @param $week
 *   A week object.
 * @return boolean
 *   TRUE if league contains week, FALSE otherwise.
 */
function is_valid_week($league, $week) {
  $query = "SELECT lid, wid from {pickem_leagues} l inner join {pickem_weeks} w on w.gsid=l.gsid WHERE lid=%d and wid=%d";
  $result = db_query($query, $league->lid, $week->wid);
  $row = db_fetch_object($result);
  if ( isset($row->lid) ) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

function is_last_game_of_week($game, $reset = FALSE) {
  static $lastgames = array();
  $cachekey = $game->wid;
  if (empty($lastgames[$cachekey])) {
    $sql = "SELECT gid from {pickem_games} where wid = ". $game->wid ." order by gametime desc, gid";
    $result = db_query_range($sql, 0, 1);
    $row = db_fetch_object($result);
    $lastgames[$cachekey] = $row->gid;
  }
  return $game->gid == $lastgames[$cachekey] ? TRUE : FALSE;
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


function get_users($league=NULL, $not_in_league=0, $full_user_load=0) {

  if ( !isset($league) ) {
    $query = "SELECT u.* from {users} u where u.uid<>0 order by name";
    $result = db_query($query);
  }
  else {
    if ( $not_in_league == 0 ) {
      //drupal_set_message("All users IN league " . $league->lid);
      $query = "SELECT u.* from {users} u inner join {pickem_users} pu on pu.uid=u.uid where u.uid<>0 and u.status<>0 and pu.lid=%d order by name";
    }
    else {
      $query = "SELECT * from {users} where uid not in (select uid from {pickem_users} where lid=%d) and uid<>0 and status<>0 order by name";
    }
    $result = db_query($query, $league->lid);
  }
  $users = array();
  while ($row = db_fetch_object($result)) {
    $users[$row->uid] = $row;
    if ( $full_user_load == 1 ) {
      // Do a full user_load, hooks and all, for RealName module compatibility.
      $users[$row->uid]->user_obj = user_load($row->uid);
    }
    else {
      // Create a user-like object.  This is more performant on large user lists than a full user_load().
      $users[$row->uid]->user_obj = new stdClass;
      $users[$row->uid]->user_obj->name = $row->name;
      $users[$row->uid]->user_obj->uid = $row->uid;
    }
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
function get_my_leagues($gameset_id=NULL, $user_id=NULL) {

  // use the logged in user if no user_id is passed in.
  if ( is_null($user_id) ) {
    global $user;
    $user_id = $user->uid;
  }

  if ( is_null($gameset_id) ) {
    $query = "SELECT l.* from {pickem_leagues} l inner join {pickem_users} u on u.lid=l.lid where u.uid=%d";
    $result = db_query($query, $user_id);
  }
  else {
    $query = "SELECT l.* from {pickem_leagues} l inner join {pickem_users} u on u.lid=l.lid where u.uid=%d and l.gsid=%d";
    $result = db_query($query, $user_id, $gameset_id);
  }
  $leagues = array();
  while ($row = db_fetch_object($result)) {
    $leagues[] = $row;
  }
  return $leagues;
}


/**
 * Perform a check if a user is a member of a particular league.
 *
 * @param $league
 *   A league object.
 * @param $account
 *   A user object.
 * @return boolean
 *   TRUE if current user is a league member, FALSE otherwise.
 */
function is_league_member($league, $account) {
  $query = "SELECT l.* from {pickem_leagues} l inner join {pickem_users} pu on pu.lid=l.lid where pu.lid=%d and pu.uid=%d";
  $row = db_fetch_object(db_query($query, $league->lid, $account->uid));
  if ( $row ) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}


/**
 * Perform a check if the current user is a member of a particular league.
 *
 * @param $league
 *   A league object.
 * @return boolean
 *   TRUE if current user is a league member, FALSE otherwise.
 */
function is_my_league($league) {
  global $user;
  return is_league_member($league, $user);
}


function get_teams($filter = array('team_id' => NULL, 'gameset_id' => NULL)) {
  if ( !is_null($filter['gameset_id']) ) {
    $query = "SELECT * from {pickem_teams} where gsid=%d order by tm_abbrev, tm_name";
    $result = db_query($query, $filter['gameset_id']);
  }
  elseif ( !is_null($filter['team_id']) ) {
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
      $row->small_logo_path = "/$module_path/logos/". $row->logo_small;
      $row->small_logo_img = '<img src="' . $row->small_logo_path . '" />';
    }
    else {
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

function get_games($filter=array('game_id' => NULL, 'week_id' => NULL, 'gameset_id' => NULL, 'team_id' => NULL, 'league_id' => NULL), $options=array('by_week' => 0, 'order_by' => 'week_past, g.gametime, g.gid') ) {

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
  $order_by = 'week_past, g.gametime, g.gid';
  if ( !is_null($options['order_by']) ) {
    $order_by = $options['order_by'];
  }


  $sql_where = '';
  if ( count($sql_where_list) > 0 ) {
    $sql_where = ' WHERE '. implode(' and ', $sql_where_list);
  }

  $games = array();
  $mysqltime = date("Y-m-d H:i:s");

  if ( is_null($filter['league_id'] ) ) {
    $query = "SELECT g.*, case when '$mysqltime' > pw.wk_firstgame then 1 else 0 end as 'week_past', case when '$mysqltime' > date_add(pw.wk_firstgame, INTERVAL 7 DAY) then 1 else 0 end as 'week_over', pw.wk_firstgame, pw.wk_number FROM {pickem_games} g INNER JOIN {pickem_weeks} pw ON pw.wid=g.wid  ". $sql_where ." ORDER BY $order_by";
  }
  else {
    $query = "SELECT g.*, case when '$mysqltime' > pw.wk_firstgame then 1 else 0 end as 'week_past', case when '$mysqltime' > date_add(pw.wk_firstgame, INTERVAL 7 DAY) then 1 else 0 end as 'week_over', pw.wk_firstgame, pw.wk_number FROM {pickem_games} g INNER JOIN {pickem_weeks} pw ON pw.wid=g.wid inner join {pickem_leagues} l on l.gsid=pw.gsid ". $sql_where ." ORDER BY $order_by";
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

  $query = 'SELECT g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.h_id, p.uid, p.winnerpick_id, p.pick_v_score, p.pick_h_score, p.pick_v_score, p.pick_h_score, g.gametime, w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid inner join {pickem_leagues} l on l.gsid=w.gsid and l.lid=u.lid '. $sql_where .' ORDER BY g.wid, g.gametime, g.gid';
  $result = db_query($query);

  $picks = array();
  while ($row = db_fetch_object($result)) {
    $picks[$row->gid][$row->uid] = $row;

    // If the user did not pick the winner, but did pick the score, use the score values.
    if ( !isset($picks[$row->gid][$row->uid]->winnerpick_id) && isset($row->pick_v_score) && isset($row->pick_h_score) ) {
      $picks[$row->gid][$row->uid]->winnerpick_id = $row->pick_h_score > $row->pick_v_score ? $row->h_id : $row->v_id;
    }
  }

  return $picks;
}
function set_pick($user_id, $league, $pick) {

  // Figure out if we need to insert or update.
  $query = 'select count(*) as pickcount from {pickem_picks} where gid=%d and uid=%d and lid=%d';
  $result = db_query($query, $pick['gid'], $user_id, $league->lid);
  if ($row = db_fetch_object($result)) {
    if ($row->pickcount > 0) {
      $type = "update";
    }
    else {
      $type = "insert";
    }
  }

  // Pick the winner.
  if (isset($pick['pick'])) {
    if ($type == "insert") {
      drupal_set_message("insert");
      $query = "insert into {pickem_picks} (gid,uid,lid,winnerpick_id,picktime) values (%d,%d,%d,%d,'%s')";
      $result = db_query($query, $pick['gid'], $user_id, $league->lid, $pick['pick'], date('Y-m-d H:i:s', time()));
      if ( $result == FALSE ) {
        drupal_set_message('pickem', 'set_pick insert failed', 'error');
      }
    }
    elseif ($type == "update") {
      $query = "update {pickem_picks} set winnerpick_id=%d,picktime='%s' where gid=%d and uid=%d and lid=%d";
      $result = db_query($query, $pick['pick'], date('Y-m-d H:i:s', time()), $pick['gid'], $user_id, $league->lid);
      if ( $result == FALSE ) {
        drupal_set_message('pickem', 'set_pick update failed', 'error');
      }
    }
  }

  // Pick the score.
  else {
    if ($type == "insert") {
      $query = "insert into {pickem_picks} (gid,uid,lid,pick_v_score,pick_h_score,picktime) values (%d,%d,%d,%d,%d,'%s')";
      $result = db_query($query, $pick['gid'], $user_id, $league->lid, $pick['pick_v_score'], $pick['pick_h_score'], date('Y-m-d H:i:s', time()));
      if ( $result == FALSE ) {
        drupal_set_message('pickem', 'set_pick insert failed', 'error');
      }
    }
    elseif ($type == "update") {
      // NOTE: using db_query with replacements, you can't set the score to NULL from the GUI.  A blank is cast to 0.
      $query = "update {pickem_picks} set pick_v_score=%d, pick_h_score=%d, picktime='%s' where gid=%d and uid=%d and lid=%d";
      $result = db_query($query, $pick['pick_v_score'], $pick['pick_h_score'], date('Y-m-d H:i:s', time()), $pick['gid'], $user_id, $league->lid);
      if ( $result == FALSE ) {
        drupal_set_message('pickem', 'set_pick update failed', 'error');
      }
    }
  }

}

/**
 * Fetch an array containing user point scores for the league.
 * Long running query, uses static variable cache and DB cache where it can.
 * DB cache should be cleared when game scores are updated.
 *
 * @param $league
 *   A $league object for the league in question.
 * @param $week_id
 *   A numeric week id (wid).
 * @param $reset
 *   A boolean for resetting the static variable containing cached return values.
 * @return
 *   A complex array, keyed with uid, containing the users point totals, overall and by week.
 */
function get_user_totals($league=NULL, $week_id=0, $reset = FALSE) {
  static $cache_totals;
  $cache_key = 'pickem_get_user_totals_'. $league->lid .'_'. $week_id;
  if (!isset($cache_totals[$cache_key]) || $reset) {
    if (!$reset && ($cache = cache_get($cache_key)) && !empty($cache->data)) {
      $totals = $cache->data;
    }
    else {

      // Load the scoring engine include file and test for the get_user_totals_* function.
      require_once $league->scoring_engine['include'];
      if (!isset($league->scoring_engine) || !function_exists('get_user_totals_'. $league->scoring_engine['type'])) {
        drupal_set_message('No scoring engine.');
        return;
      }

      // Initialized $totals array with current league users.
      $totals = array();
      $users = get_users($league, 0, variable_get('pickem_full_user_load', 0));
      foreach ($users as $u) {
        $totals[$u->uid] = array(
          'user_obj' => $u,
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
        $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.v_spread, g.h_id, g.h_spread, p.uid, p.winnerpick_id, p.pick_v_score, p.pick_h_score, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wk_firstgame < '". $mysqltime ."' order by g.wid, g.gametime, g.gid";
        $result = db_query($query, $league->lid);
      }
      else {
        // all games for a certain week
        $query = "select g.gid, g.wid, g.h_score, g.v_score, g.v_id, g.v_spread, g.h_id, g.h_spread, p.uid, p.winnerpick_id, p.pick_v_score, p.pick_h_score, g.gametime,w.wk_points from {pickem_users} u inner join {pickem_picks} p on u.lid=p.lid and u.uid=p.uid inner join {pickem_games} g on p.gid = g.gid  inner join {pickem_weeks} w on w.wid=g.wid where p.lid=%d and w.wid=%d order by g.wid, g.gametime, g.gid";
        $result = db_query($query, $league->lid, $week_id);
      }

      while ($row = db_fetch_object($result)) {
        // If user is not in list, skip them.
        if (!isset($totals[$row->uid])) {
          continue;
        }

        if (!isset ($totals[$row->uid]['weeks'][$row->wid])) {
          $totals[$row->uid]['weeks'][$row->wid]['total'] = 0;
        }

        // Calculate weekly totals using the appropriate scoring engine.
        call_user_func('get_user_totals_'. $league->scoring_engine['type'], &$totals, $row);

      }

      // Calculate user's best and worst weeks.
      $weekwinners = array();
      foreach ($totals as $uid => $u_object) {
        foreach ($u_object['weeks'] as $wid => $week_item) {
          if ( $totals[$uid]['best_week_id'] == -1 ) {
            $totals[$uid]['best_week_id'] = $wid;
            $totals[$uid]['worst_week_id'] = $wid;
          }
          if ( $week_item['total'] > $totals[$uid]['weeks'][$totals[$uid]['best_week_id']]['total'] ) {
            $totals[$uid]['best_week_id'] = $wid;
          }
          if ( $week_item['total'] < $totals[$uid]['weeks'][$totals[$uid]['worst_week_id']]['total'] ) {
            $totals[$uid]['worst_week_id'] = $wid;
          }
        }
      }

      // Calculate total ordinal ranking.
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
      cache_set($cache_key, $totals);
    }
    $cache_totals[$cache_key] = $totals;
  }

  return $cache_totals[$cache_key];
}
function sort_totals($a, $b) {
  if ($a['total'] > $b['total']) {
    return -1;
  }
  elseif ($a['total'] < $b['total']) {
      return 1;
  }
  else {
    if ($a['name'] < $b['name']) {
      return -1;
    }
    elseif ($a['name'] > $b['name']) {
        return 1;
    }
    else {
      return 0;
    }
  }
}

/**
 * Fetch an array containing current week details
 * Often called query, but it is fast enought to not require caching.
 *
 * @param $league
 *   A $league object for the league in question.
 * @param $nextweek
 *   A boolean containing the sort item.
 * @return
 *   A complex array, keyed with team id(tid), containing the teams statistics, overall and by week.
 */
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
  $row->firstgame_xml = $fg[0] .'T'. $fg[1] . $tz_offset;

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
      $week_total = 0;
      $count = 0;
      foreach ($totals as $uid => $tot) {
        if ( isset($tot['weeks'][$wid]['total']) ) {
          if ($tot['weeks'][$wid]['total'] > $max) {
            $max = $tot['weeks'][$wid]['total'];
          }
          if ( $tot['weeks'][$wid]['total'] < $min ) {
            $min = $tot['weeks'][$wid]['total'];
          }
          $week_total += $tot['weeks'][$wid]['total'];
          $count += 1;
        }
      }

      $wk_summary[$wid] = array(
        "name" => $week->wk_number,
        "max" => $max,
        "min" => $min,
        "count" => $count,
        "total" => $week_total,
      );
    }
  }
  return $wk_summary;
}


/**
 * Fetch an array containing team win-loss and other statistics.
 * Long running query, uses static variable cache and DB cache where it can.
 * DB cache should be cleared when game scores are updated.
 *
 * @param $league
 *   A $league object for the league in question.
 * @param $sort_by
 *   A string containing the sort item.
 * @param $week_id
 *   A numeric week id (wid).
 * @param $reset
 *   A boolean for resetting the static variable containing cached return values.
 * @return
 *   A complex array, keyed with team id(tid), containing the teams statistics, overall and by week.
 */
function get_team_totals($league, $sort_by='win_lose', $week_id=0, $reset = FALSE) {
  static $cache_totals;
  $cache_key = 'pickem_get_team_totals_'. $league->lid .'_'. $sort_by .'_'. $week_id;

  if (!isset($cache_totals[$cache_key]) || $reset) {
    if (!$reset && ($cache = cache_get($cache_key)) && !empty($cache->data)) {
      $totals = $cache->data;
    }
    else {
      $totals = array();
      $teams = get_teams(array('gameset_id' => $league->gsid));

      foreach ($teams as $tid => $team) {
        $totals[$tid] = array(
          'total_picks' => 0,
          'win_picks' => 0,
          'correct_picks' => 0,
          'pickability' => NULL,
          'pickability_w' => NULL,
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
        elseif ($row->v_score > $row->h_score && $row->winnerpick_id == $row->v_id) {
          $totals[$row->v_id]['correct_picks'] += 1;
          $totals[$row->v_id]['win_picks'] += 1;
          $totals[$row->h_id]['correct_picks'] += 1;
          $totals[$row->v_id]['wins'] += 1;
        }
        // Tie
        elseif ($row->v_score == $row->h_score && $row->h_score <> "") {
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
      cache_set($cache_key, $totals);
    }
    $cache_totals[$cache_key] = $totals;
  }

  return $cache_totals[$cache_key];
}


function sort_win_lose($a, $b) {
  if ( $a['pickability'] > $b['pickability'] || is_null($b['pickability']) ) {
    return -1;
  }
  elseif ( $a['pickability'] < $b['pickability'] || is_null($a['pickability']) ) {
      return 1;
  }
  else {
    if ( $a['abbrev'] < $b['abbrev'] ) {
      return -1;
    }
    elseif ($a['abbrev'] > $b['abbrev']) {
        return 1;
    }
    else {
      return 0;
    }
  }
}
function sort_to_win($a, $b) {
  if ( $a['pickability_w'] > $b['pickability_w'] || is_null($b['pickability_w']) ) {
    return -1;
  }
  elseif ($a['pickability_w'] < $b['pickability_w'] || is_null($a['pickability_w']) ) {
      return 1;
  }
  else {
    if ($a['abbrev'] < $b['abbrev']) {
      return -1;
    }
    elseif ($a['abbrev'] > $b['abbrev']) {
        return 1;
    }
    else {
      return 0;
    }
  }
}


function get_team_stats($league) {

  // team_stats will contain rolling totals for each week.
  $team_stats = array();

  // default values to initialize team data (gets copied)
  $team_def = array(
    'team' => NULL,
    'wins' => 0,
    'losses' => 0,
    'ties' => 0,
    'streak' => 0,
    'games_played' => 0,
    'pts_for' => 0,
    'pts_against' => 0,
    'win_percent' => NULL,

    'opp_win_sum' => 0,
    'scoring_sum' => 0,
    'ord' => 0
  );

  // $team_stats[0] is where the current totals are stored
  $teams = get_teams(array('gameset_id' => $league->gsid));
  foreach ($teams as $tid => $t) {
    // make sure both teams are initialized
    if ( !isset($team_stats[0][$tid]) ) {
      $team_stats[0][$tid] = $team_def;
      $team_stats[0][$tid]['team'] = $t;
    }
  }

  // loop through the games and get the data
  $games = get_games(array('gameset_id' => $league->gsid), array('order_by' => 'pw.wk_firstgame,g.gametime,g.gid'));
  foreach ($games as $gid => $game) {
    // skip unplayed games
    if ( ! isset($game->h_score) || ! isset($game->v_score) ) {
      continue;
    }


    /*
     * get the stats
     */

    // home win
    if ( $game->h_score > $game->v_score ) {
      $team_stats[0][$game->h_id]['wins']++;
      $team_stats[0][$game->v_id]['losses']++;

      // streak calc
      if ( $team_stats[0][$game->h_id]['streak'] >= 0 ) {
        $team_stats[0][$game->h_id]['streak']++;
      } else {
        $team_stats[0][$game->h_id]['streak'] = 1;
      }
      if ( $team_stats[0][$game->v_id]['streak'] <= 0 ) {
        $team_stats[0][$game->v_id]['streak']--;
      } else {
        $team_stats[0][$game->v_id]['streak'] = -1;
      }

    }

    // visitor win
    elseif ( $game->h_score < $game->v_score ) {
      $team_stats[0][$game->v_id]['wins']++;
      $team_stats[0][$game->h_id]['losses']++;

      // streak calc
      if ( $team_stats[0][$game->v_id]['streak'] >= 0 ) {
        $team_stats[0][$game->v_id]['streak']++;
      } else {
        $team_stats[0][$game->v_id]['streak'] = 1;
      }
      if ( $team_stats[0][$game->h_id]['streak'] <= 0 ) {
        $team_stats[0][$game->h_id]['streak']--;
      } else {
        $team_stats[0][$game->h_id]['streak'] = -1;
      }

    }

    // tie
    elseif ( $game->h_score == $game->v_score ) {
      $team_stats[0][$game->v_id]['ties']++;
      $team_stats[0][$game->h_id]['ties']++;

      // does not affect streak
    }

    $team_stats[0][$game->v_id]['games_played']++;
    $team_stats[0][$game->h_id]['games_played']++;

    $team_stats[0][$game->v_id]['pts_for'] += $game->v_score;
    $team_stats[0][$game->v_id]['pts_against'] += $game->h_score;

    $team_stats[0][$game->h_id]['pts_for'] += $game->h_score;
    $team_stats[0][$game->h_id]['pts_against'] += $game->v_score;

    // Win Percentage
    if ( $team_stats[0][$game->h_id]['wins'] + $team_stats[0][$game->h_id]['losses'] > 0 ) {
      $team_stats[0][$game->h_id]['win_percent'] = $team_stats[0][$game->h_id]['wins'] / ( $team_stats[0][$game->h_id]['wins'] + $team_stats[0][$game->h_id]['losses'] );
    }
    if ( $team_stats[0][$game->v_id]['wins'] + $team_stats[0][$game->v_id]['losses'] > 0 ) {
      $team_stats[0][$game->v_id]['win_percent'] = $team_stats[0][$game->v_id]['wins'] / ( $team_stats[0][$game->v_id]['wins'] + $team_stats[0][$game->v_id]['losses'] );
    }

    // copy into weeks stats
    $team_stats[$game->wid][$game->h_id] = $team_stats[0][$game->h_id];
    $team_stats[$game->wid][$game->v_id] = $team_stats[0][$game->v_id];

  }

  return $team_stats;
}


function build_picks_week_form($form_state=NULL, $league, $week, $account) {

  $teams = get_teams(array('gameset_id' => $league->gsid));
  $games = get_games(array('week_id' => $week->wid));
  $picks = get_picks(array('league_id' => $league->lid, 'user_id' => $account->uid, 'week_id' => $week->wid));

  $form = array();
  $form['games'] = array('#tree' => TRUE);
  $last = count($games);
  $count = 0;
  foreach ($games as $gid => $game) {

    $count++;
    $form['games'][$gid]['v_id'] = array(
      '#type' => 'hidden',
      '#value' => $game->v_id
    );
    $form['games'][$gid]['h_id'] = array(
      '#type' => 'hidden',
      '#value' => $game->h_id
    );

    // Build-up the team labels.
    $v_team_label = $teams[$game->v_id]->small_logo_img . $teams[$game->v_id]->short_name;
    $h_team_label = $teams[$game->h_id]->small_logo_img . $teams[$game->h_id]->short_name;
    if ( $league->scoring_engine['features']['with_spread'] == 1 ) {
      $v_team_label .= '&nbsp;&nbsp;' . ($game->v_spread>0?'+'.$game->v_spread:$game->v_spread);
      $h_team_label .= '&nbsp;&nbsp;' . ($game->h_spread>0?'+'.$game->h_spread:$game->h_spread);
    }


    // Add the pick score boxes.
    if ( $league->scoring_engine['features']['pick_score'] == 1 ||
      ($league->scoring_engine['features']['with_last_weekly_score'] == 1 && $count == $last) ) {
      $form['games'][$gid]['pick_v_score'] = array(
        '#type' => 'textfield',
        '#size' => 4,
        '#default_value' => $picks[$gid][$account->uid]->pick_v_score,
        '#field_suffix' => $v_team_label,
        '#prefix' => '<div class="pickem-team-score-diff">'
      );
      $form['games'][$gid]['pick_h_score'] = array(
        '#type' => 'textfield',
        '#size' => 4,
        '#default_value' => $picks[$gid][$account->uid]->pick_h_score,
        '#field_suffix' => $h_team_label,
        '#suffix' => '</div>'
      );

      // Disabled the input boxes if the week has past.
      if ( $week->week_past == 1 ) {
        $form['games'][$gid]['pick_v_score']['#attributes']['readonly'] = 'readonly';
        $form['games'][$gid]['pick_v_score']['#attributes']['disabled'] = 'disabled';
        $form['games'][$gid]['pick_h_score']['#attributes']['readonly'] = 'readonly';
        $form['games'][$gid]['pick_h_score']['#attributes']['disabled'] = 'disabled';
      }

    // Add the pick team boxes.
    } else {
      $form['games'][$gid]['pick'] = array(
        '#type' => 'radios',
        '#default_value' => $picks[$gid][$account->uid]->winnerpick_id,
        '#attributes' => array('class' => 'pickem-team-radios'),
        '#options' => array($game->v_id=>$v_team_label, $game->h_id=>$h_team_label)
      );

      // Disabled the radio boxes if the week has past.
      if ( $week->week_past == 1 ) {
        $form['games'][$gid]['pick']['#attributes']['readonly'] = 'readonly';
        $form['games'][$gid]['pick']['#attributes']['disabled'] = 'disabled';
      }

    }

    $form['games'][$gid]['gametime'] = array(
      '#value' => $game->gametime
    );
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

  $form['apply_to_all_leagues'] = array(
    '#type' => 'checkbox',
    '#access' => ($week->week_past == 0 && count($apply_to_leagues) > 1),
    '#title' => t('Apply picks to all leagues that use the same game set and scoring system.'),
    '#description' => implode(', ', $apply_to_leagues),
    '#default_value' => 0
  );


  // Add the submit button.
  $form['submit'] = array(
    '#type' => 'submit',
    '#access' => ($week->week_past == 0),
    '#value' => t('Save Picks'),
    '#weight' => 5
  );

  return $form;
}