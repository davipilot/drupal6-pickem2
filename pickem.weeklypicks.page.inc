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
    $output .= '<th>'. $teams[$g_data->v_id]->tm_abbrev ."<br/>@<br/>". $teams[$g_data->h_id]->tm_abbrev .'</th>';
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

      if ($league->scoring_type == PICKEM_TYPE_WIN_LOSS || $league->scoring_type == PICKEM_TYPE_SPREAD) {
        $p_team = $teams[$picks[$gid][$uid]->winnerpick_id]->short_name;

        if (isset ($picks[$gid][$uid]->winnerpick_id) && $picks[$gid][$uid]->winnerpick_id == $winnerid && $winnerid != 0) {
          $output .= '<td class="win">'. $p_team .'</td>';
        }
        elseif (isset ($picks[$gid][$uid]->winnerpick_id) && $winnerid == -1) {
          $output .= '<td class="tie">'. $p_team .'</td>';
        }
        elseif (isset ($picks[$gid][$uid]->winnerpick_id) && $picks[$gid][$uid]->winnerpick_id != $winnerid && $winnerid != 0) {
          $output .= '<td class="loss">'. $p_team .'</td>';
        }
        else {
          if ($week->week_past == 1 || $uid == $user->uid) {
            $output .= '<td>'. $p_team .'</td>';
          }
          else {
            if (isset ($picks[$gid][$uid]->winnerpick_id)) {
              $output .= '<td>X</td>';
            }
            else {
              $output .= '<td>&nbsp;</td>';
            }
          }
        }
      }
      else if ($league->scoring_type == PICKEM_TYPE_SCORE_DIFF || $league->scoring_type == PICKEM_TYPE_WIN_LOSS_TIEBREAK) {
        $output .= '<td>'. $tot['weeks'][$week->wid]['games'][$gid] .'</td>';
      }



    }
    $output .= '</tr>';
  }

  $output .= '</table></div>';

  drupal_set_title($week->wk_name .' Pick Summary');
  return $output;
}
