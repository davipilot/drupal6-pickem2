<?php
// $Id: $

/**
 * @file
 * Point spread scoring engine include file.
 * Functions in here should have the _spread extension for avoid clashing with other scoring engine namespaces.
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
function get_user_totals_spread($totals=FALSE, $row=FALSE) {

  // home win
  if ( ($row->h_score + $row->h_spread) > $row->v_score && $row->winnerpick_id == $row->h_id) {
    $totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid] = $row->wk_points;
    $totals[$row->uid]['weeks'][$row->wid]['total'] += $row->wk_points;
    $totals[$row->uid]['total'] += $row->wk_points;
  }
  // visitor win
  elseif ( ($row->v_score + $row->v_spread) > $row->h_score && $row->winnerpick_id == $row->v_id) {
    $totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid] = $row->wk_points;
    $totals[$row->uid]['weeks'][$row->wid]['total'] += $row->wk_points;
    $totals[$row->uid]['total'] += $row->wk_points;
  }
  // tie.
  elseif ( ($row->v_score + $row->v_spread) == $row->h_score && $row->h_score <> "") {
    $totals[$row->uid]['weeks'][$row->wid]['games'][$row->gid] = $row->wk_points;
    $totals[$row->uid]['weeks'][$row->wid]['total'] += $row->wk_points;
    $totals[$row->uid]['total'] += $row->wk_points;
  }

}
