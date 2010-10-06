<?php
// $Id: $

/**
 * @file
 * Page callbacks and theme functions for the weekly picks page.
 */

function weeklypicks_page($league, $week) {
  global $user;

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l($league->name, 'pickem/'. $league->lid);
  drupal_set_breadcrumb($breadcrumb);

  $flip = array('even' => 'odd', 'odd' => 'even');
  $class = 'even';

  $output = '<div class="pickem">Sorted by this week\'s scores<table class="pick-summary"><tr><th>Weekly Rank</th><th>Points</th>';
  $gids = array();
  $teams = get_teams(array('gameset_id' => $league->gsid));
  $weeks = get_weeks(array('week_id' => $week->wid));
  $games = get_games(array('week_id' => $week->wid));
  foreach ($games as $gid => $g_data) {
    if (variable_get('pickem_prefer_long_team_names', 0) == 1) {
      $output .= '<th>'. $teams[$g_data->v_id]->long_name ."<br/>@<br/>". $teams[$g_data->h_id]->long_name .'</th>';
    }
    else {
      $output .= '<th>'. $teams[$g_data->v_id]->short_name ."<br/>@<br/>". $teams[$g_data->h_id]->short_name .'</th>';
    }
    if ( $g_data->h_score == NULL ) {
      $gids[$gid] = 0;  // game not played
    }
    elseif ($g_data->h_score == $g_data->v_score) {
      $gids[$gid] = -1; // game ended in tie
    }
    elseif ($g_data->h_score > $g_data->v_score) {
      $gids[$gid] = $g_data->h_id;
    }
    elseif ($g_data->v_score > $g_data->h_score) {
      $gids[$gid] = $g_data->v_id;
    }
  }
  $output .= '</tr>';

  $totals = get_user_totals($league, $week->wid); //sorted by weekly scores
  $picks = get_picks( array('league_id' => $league->lid, 'week_id' => $week->wid) );
  foreach ($totals as $uid => $tot) {
    $class=$flip[$class];
    $output .= '<tr class="'. $class .'"><td class="users">'. $tot['ranking'] .'. '. theme('username', $tot['user_obj']) .'</td><td>'. $tot['total'] .'</td>';
    foreach ($gids as $gid => $winnerid) {

      // Week has not yet started, don't show picks or scores unless auth user is the pick user.
      if ($week->week_past == 0) {
        if (isset ($picks[$gid][$uid]->winnerpick_id)) {
          $p_team = $teams[$picks[$gid][$uid]->winnerpick_id]->short_name;
          $output .= ($uid == $user->uid) ? '<td>'. $p_team .'</td>' : '<td>X</td>';
        }
        else {
          $output .= '<td>&nbsp;</td>';
        }
      }

      // Week start has passed, go ahead and show picks and totals.
      else {
        $p_team = $teams[$picks[$gid][$uid]->winnerpick_id]->short_name;

        // If user received some points, mark it green (class=win).
        if ($tot['weeks'][$week->wid]['games'][$gid] > 0) {
          $output .= '<td class="win">'. $p_team .': ' . $tot['weeks'][$week->wid]['games'][$gid] . '</td>';
        }
        elseif ($tot['weeks'][$week->wid]['games'][$gid] == 0 && $gids[$gid] != 0) {
          $output .= '<td class="loss">'. $p_team .': ' . $tot['weeks'][$week->wid]['games'][$gid] . '</td>';
        }
        else {
          $output .= '<td>'. $p_team . '</td>';
        }
      }
    }
    $output .= '</tr>';
  }

  $output .= '</table></div>';

  drupal_set_title(t('Pick Summary - ') . $week->wk_name);
  return $output;
}
