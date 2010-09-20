<?php
// $Id: $

/**
 * @file
 * Win-Loss-Tiebreak scoring engine include file.
 * Functions in here should have the _win_loss_tiebreak extension to avoid clashing with other scoring engine namespaces.
 * Scoring engines require a get_user_totals_* function.
 */


/**
 * Calculate user totals for this scoring system.
 *
 * @param $totals
 *   A deep array to store user totals (from get_user_totals() in pickem.inc)
 * @param $row
 *   A row object consisting of game and pick data (from get_user_totals() in pickem.inc)
 * @return
 *   NULL
 */
function get_user_totals_win_loss_tiebreak($totals=FALSE, $row=FALSE) {

  // If it is the last game of the week, use the score picks.
  if ( is_last_game_of_week(array('gid'=>$row->gid, 'wid'=>$row->wid)) ) {

    // Picked the correct winner or tie (1 point).
    if ($row->h_score > $row->v_score && $row->pick_h_score > $row->pick_v_score ||
        $row->v_score > $row->h_score && $row->pick_v_score > $row->pick_h_score ||
        $row->v_score == $row->h_score && $row->pick_v_score == $row->pick_h_score) {
      $totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid] = 1;
      $totals[$row->uid]['weeks'][$row->wid]['total'] += 1;
      $totals[$row->uid]['total'] += 1;
      $totals[$row->uid]['weeks'][$row->wid]['picks'][$row->gid] = 1;
    }

    // Picked the correct score differential.
    if ($row->h_score - $row->v_score == $row->pick_h_score - $row->pick_v_score) {
      $totals[$row->uid]['weeks'][$row->wid]['picks'][$row->gid] += 1;
    }

    // Picked the exact match score.
    if ($row->h_score == $row->pick_h_score && $row->v_score == $row->pick_v_score) {
      $totals[$row->uid]['weeks'][$row->wid]['picks'][$row->gid] += 2;
    }
  }

  // If it is not the last game, use normal pick the winner id, and don't use the pick scores at all.
  else {
    if ($row->h_score > $row->v_score && $row->winnerpick_id == $row->h_id) {
      $totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid] = $row->wk_points;
      $totals[$row->uid]['weeks'][$row->wid]['total'] += $row->wk_points;
      $totals[$row->uid]['total'] += $row->wk_points;
    }
    elseif ($row->v_score > $row->h_score && $row->winnerpick_id == $row->v_id) {
      $totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid] = $row->wk_points;
      $totals[$row->uid]['weeks'][$row->wid]['total'] += $row->wk_points;
      $totals[$row->uid]['total'] += $row->wk_points;
    }
    elseif ($row->v_score == $row->h_score && $row->h_score <> "") {
      $totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid] = $row->wk_points;
      $totals[$row->uid]['weeks'][$row->wid]['total'] += $row->wk_points;
      $totals[$row->uid]['total'] += $row->wk_points;
    }
  }

  // If the user received no points for this game, put in a zero.
  if (! isset($totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid])) {
    $totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid] = 0;
  }

}

/**
 * After all user totals are calculated, this function is called to wrap-up any extra calculations.
 *
 * @param $totals
 *   A deep array to store user totals (from get_user_totals() in pickem.inc)
 * @return
 *   NULL
 */
function get_user_totals_finalize_win_loss_tiebreak($totals=FALSE) {

  // Get the weekly high-scores and users who achieved it.
  $weekly_best = array();
  foreach ($totals as $uid => $u_object) {
    foreach ($u_object['weeks'] as $wid => $w_object) {
      if (!isset($weekly_best[$wid])) {
        $weekly_best[$wid] = array('pts'=>0, 'users'=>array());
      }
      // If this is a new best score, set the weekly_best and clear the previous users.
      if ($w_object['total'] > $weekly_best[$wid]['pts']) {
        $weekly_best[$wid]['pts'] = $w_object['total'];
        $weekly_best[$wid]['users'] = array();
        $weekly_best[$wid]['users'][$uid] = 1;
      }
      elseif ($w_object['total'] == $weekly_best[$wid]['pts']) {
        $weekly_best[$wid]['users'][$uid] = 1;
      }
    }
  }

  foreach ($totals as $uid => $u_object) {
    foreach ($u_object['weeks'] as $wid => $w_object) {
      // Skip the user-week if they didn't get the best score that week.
      // Or there was only one best score that week => NO BONUS.
      if ( !isset($weekly_best[$wid]['users'][$uid]) || count($weekly_best[$wid]['users']) < 2 ) {
        continue;
      }

      foreach ($w_object['games'] as $gid => $g_object) {
        if ( is_last_game_of_week(array('gid'=>$gid, 'wid'=>$wid)) ) {
         // Picked the correct score differential (1 more point).
          if ($totals[$uid]['weeks'][$wid]['picks'][$gid] == 2) {
            $totals[$uid]['weeks'][$wid]['games'][$gid] += 1;
            $totals[$uid]['weeks'][$wid]['total'] += 1;
            $totals[$uid]['total'] += 1;
          }

          // Picked the exact match score (2 more points).
          if ($totals[$uid]['weeks'][$wid]['picks'][$gid] == 4) {
            $totals[$uid]['weeks'][$wid]['games'][$gid] += 2;
            $totals[$uid]['weeks'][$wid]['total'] += 2;
            $totals[$uid]['total'] += 2;
          }

        }
      }
    }
  }

}
