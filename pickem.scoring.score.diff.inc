<?php
// $Id: $

/**
 * @file
 * Score differential scoring engine include file.
 * Functions in here should have the _score_diff extension to avoid clashing with other scoring engine namespaces.
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
function get_user_totals_score_diff($totals=FALSE, $row=FALSE) {

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

  // If the user received no points for this game, put in a zero.
  if (! isset($totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid])) {
    $totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid] = 0;
  }

}
