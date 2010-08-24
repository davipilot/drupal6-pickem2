<?php
// $Id: $

/**
 * @file
 * Page callbacks and theme functions for the team power page.
 */


function team_power_page($league) {
  drupal_set_title('Team vPower Rankings');

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l($league->name, 'pickem/'. $league->lid);
  drupal_set_breadcrumb($breadcrumb);

  $o = '<div class="pickem">'. theme('team_power', $league) .'</div>';

  print theme('page', $o);
}

function theme_team_power($league) {

  $team_stats = get_team_stats($league);

  $games = get_games(array('gameset_id' => $league->gsid), array('order_by' => 'pw.wk_firstgame,g.gametime,g.gid'));
  foreach ($games as $gid => $game) {
    // skip unplayed games
    if ( !isset($game->h_score) || !isset($game->v_score) ) {
      continue;
    }

    // opponents overall win percentage, not weekly win percentage
    $team_stats[0][$game->v_id]['opp_win_sum'] += $team_stats[0][$game->h_id]['win_percent'];
    $team_stats[0][$game->h_id]['opp_win_sum'] += $team_stats[0][$game->v_id]['win_percent'];

    $team_stats[0][$game->v_id]['opp_win_percentage'] = $team_stats[0][$game->v_id]['opp_win_sum'] / $team_stats[0][$game->v_id]['games_played'];
    $team_stats[0][$game->h_id]['opp_win_percentage'] = $team_stats[0][$game->h_id]['opp_win_sum'] / $team_stats[0][$game->h_id]['games_played'];

    // Scoring Factor
    if ( ($game->v_score + $game->h_score) > 0 ) {
      $team_stats[0][$game->v_id]['scoring_sum'] += $team_stats[0][$game->h_id]['win_percent'] * ($game->v_score / ($game->v_score + $game->h_score));
      $team_stats[0][$game->h_id]['scoring_sum'] += $team_stats[0][$game->v_id]['win_percent'] * ($game->v_score / ($game->v_score + $game->h_score));
    }
    $team_stats[0][$game->v_id]['scoring_factor'] = $team_stats[0][$game->v_id]['scoring_sum'] / $team_stats[0][$game->v_id]['games_played'];
    $team_stats[0][$game->h_id]['scoring_factor'] = $team_stats[0][$game->h_id]['scoring_sum'] / $team_stats[0][$game->h_id]['games_played'];

    // Streak Factor
    $streak_coef = 0.25;  // change the streak weighting
    $team_stats[0][$game->v_id]['streak_factor'] = $streak_coef * $team_stats[0][$game->v_id]['streak'] / $team_stats[0][$game->v_id]['games_played'];
    $team_stats[0][$game->h_id]['streak_factor'] = $streak_coef * $team_stats[0][$game->h_id]['streak'] / $team_stats[0][$game->h_id]['games_played'];


    // Power Ranking
    // overall visitor team power ranking
    $team_stats[0][$game->v_id]['power'] = $team_stats[0][$game->v_id]['win_percent'] + $team_stats[0][$game->v_id]['scoring_factor'] + $team_stats[0][$game->v_id]['streak_factor'];
    // weekly visitor team power ranking
    $team_stats[$game->wid][$game->v_id]['power'] = $team_stats[0][$game->v_id]['power'];
    // overall home team power ranking
    $team_stats[0][$game->h_id]['power'] = $team_stats[0][$game->h_id]['win_percent'] + $team_stats[0][$game->h_id]['scoring_factor'] + $team_stats[0][$game->h_id]['streak_factor'];
    // weekly home team power ranking
    $team_stats[$game->wid][$game->h_id]['power'] = $team_stats[0][$game->h_id]['power'];
  }


  // setup sort
  $header = array(
    array('data' => t('Team')),
    array('data' => t('WIN PCT')),
    array('data' => t('SCHEDULE')),
    array('data' => t('SCORING')),
    array('data' => t('STREAK')),
    array('data' => t('vPOWER'))
  );

  uasort($team_stats[0], 'sort_team_power');

  // collect high and low power ranking
  list($tid, $data) = each($team_stats[0]);
  $high = $data['power'];
  $low = NULL;
  while (list($tid, $data) = each($team_stats[0])) {
    $low = $data['power'];
  }
  $high = $high - $low;

  // subtract smallest power from all powers to zero base
  // multiply all powers by a factor of 100 / highest power
  reset($team_stats[0]);
  $ord = 1;
  foreach ($team_stats[0] as $tid => $data) {
    $team_stats[0][$tid]['power'] = $team_stats[0][$tid]['power'] - $low;

    // Adjusting won't work if you have a divide by zero error!
    if ( $high != 0 ) {
      $team_stats[0][$tid]['power'] = round($team_stats[0][$tid]['power'] * 100 / $high, 1);
    }

    $team_stats[0][$tid]['ord'] = $ord++;
  }


  // render team data
  $rows = array();
  foreach ($team_stats[0] as $tid => $data) {

   //print_r($data);

    $team_link = $team_stats[0][$tid]['ord'] . '. ' . $team_stats[0][$tid]['team']->long_name;

    if (module_exists("open_flash_chart_api")) {
      $team_link = l($team_link, 'pickem/'. $league->lid .'/chart3/'. $tid);
    }

    $rows[] = array(
      array('data' => $team_link),
      array('data' => sprintf("%.3f", $team_stats[0][$tid]['win_percent'])),
      array('data' => sprintf("%.3f", $team_stats[0][$tid]['opp_win_percentage'])),
      array('data' => sprintf("%.3f", $team_stats[0][$tid]['scoring_factor'])),
      array('data' => sprintf("%d", $team_stats[0][$tid]['streak']), 'style' => 'text-align:right; padding-right:20px;'),

      array('data' => $team_stats[0][$tid]['power'], 'style' => 'width:20px;')
      );
  }

  $o = theme('table', $header, $rows, array('style' => '100%;') );

  return $o;
}


function sort_team_power($a, $b) {
  if ($a['power'] > $b['power']) {
    return -1;
  }
  elseif ($a['power'] < $b['power']) {
    return 1;
  }
  else {
   return 0;
  }
}
