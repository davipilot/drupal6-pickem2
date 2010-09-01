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
  $game = new stdClass;
  $game->wid = $row->wid;
  if (is_last_game_of_week($game)) {

    // Picked the correct winner or tie (1 point).
    if ($row->h_score > $row->v_score && $row->pick_h_score > $row->pick_v_score ||
        $row->v_score > $row->h_score && $row->pick_v_score > $row->pick_h_score ||
        $row->v_score == $row->h_score && $row->pick_v_score == $row->pick_h_score) {
      $totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid] = 1;
      $totals[$row->uid]['weeks'][$row->wid]['total'] += 1;
      $totals[$row->uid]['total'] += 1;
    }

    // Picked the correct score differential (1 more point).
    if ($row->h_score - $row->v_score == $row->pick_h_score - $row->pick_v_score) {
      $totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid] += 1;
      $totals[$row->uid]['weeks'][$row->wid]['total'] += 1;
      $totals[$row->uid]['total'] += 1;
    }

    // Picked the exact match score (2 more points).
    if ($row->h_score == $row->pick_h_score && $row->v_score == $row->pick_v_score) {
      $totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid] += 2;
      $totals[$row->uid]['weeks'][$row->wid]['total'] += 2;
      $totals[$row->uid]['total'] += 2;
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
