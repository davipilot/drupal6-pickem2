<?php
// $Id: $

/**
 * @file
 * Page callbacks for display league standings.
 */


/**
 * Menu callback; presents the list of all weeks and games.
 */
function standings_page($league) {
  return theme('standings', $league);
}


/**
 * Display the league standings.
 */
function theme_standings($league) {
  drupal_set_title("Standings");

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l($league->name, 'pickem/'. $league->lid);
  drupal_set_breadcrumb($breadcrumb);

  drupal_add_js('misc/collapse.js');

  $totals = get_user_totals($league);
  $week_totals = get_totals_by_week($league);

  $flip = array('even' => 'odd', 'odd' => 'even');
  $class = 'even';

  $output = '<div class="pickem">Click on week numbers to see weekly pick summary.';
  $output .= '<div><fieldset class="collapsible"><legend><a href="#">Regular Season</a></legend>';
  $output .= '<table class="standings"><tr><th>Overall Rank</th>';

  $wk_summary = array();
  $output .= '<th colspan="2">TOT</th>';
  $weeks = get_weeks(array('gameset_id' => $league->gsid, 'playoffs' => 0));
  foreach ($weeks as $wid => $week) {
    $output .= '<th class="week-numbers">'. l($week->wk_number, 'pickem/'. $league->lid ."/summary/$wid") .'</th>';
  }
  $output .= "</tr>";

  foreach ($totals as $uid => $tot) {
    $class = $flip[$class];
    $output .= '<tr class="'. $class .'">';
    $output .= '<td class="users">'. $tot['ranking'] .'.&nbsp;&nbsp;'. theme('username', $tot['user_obj']) .'</td>';

    if ($tot['pts_back'] == 0) {
      $output .= '<td class="totals">'. $tot['total'] .'</td><td class="gamesback">&nbsp;</td>';
      $high = $tot['total'];
    }
    else {
      $output .= '<td class="totals">'. $tot['total'] .'</td><td class="gamesback">+'. $tot['pts_back'] .'</td>';
    }

    foreach ($weeks as $wid => $week) {
      if ($tot['weeks'][$wid]['total'] == $week_totals[$wid]["min"] && $week_totals[$wid]["min"] != $week_totals[$wid]["max"]) {
        $output .= '<td class="weekly weekly-low">'. $tot['weeks'][$wid]['total'] .'</td>';
      }
      elseif ($tot['weeks'][$wid]['total'] == $week_totals[$wid]["max"] && $week_totals[$wid]["min"] != $week_totals[$wid]["max"]) {
        $output .= '<td class="weekly weekly-high">'. $tot['weeks'][$wid]['total'] .'</td>';
      }
      else {
        $output .= '<td class="weekly">'. $tot['weeks'][$wid]['total'] .'</td>';
      }
    }
    $output .= "</tr>";
  }
  $output .= '</table></fieldset></div>';

  //playoff table
  $playoff_weeks = get_weeks(array('gameset_id' => $league->gsid, 'playoffs' => 1));
  if ( count($playoff_weeks) > 0 ) {
    $output .= '<div><fieldset class="collapsible"><legend><a href="#">Playoffs</a></legend>';
    $output .= '<table class="standings">';

    $output .= '<tr><th>Overall Rank</th><th colspan="2">TOT</th>';
    foreach ($playoff_weeks as $wid => $week) {
      $output .= '<th class="week-numbers">'. l('Week&nbsp;'. $week->wk_number .'<br/>'. $week->wk_name, 'pickem/'. $league->lid ."/weeklypicks/$wid", array('html' => TRUE)) .'</th>';
    }
    $output .= "</tr>";
    foreach ($totals as $uid => $u) {
      $class = $flip[$class];
      $output .= '<tr class="'. $class .'">';
      $output .= '<td class="users">'. $u['ranking'] .'.&nbsp;&nbsp;'. theme('username', $u['user_obj']) .'</td>';

      if ($u['pts_back'] == 0) {
        $output .= '<td class="totals">'. $u['total'] .'</td><td class="gamesback">&nbsp;</td>';
        $high = $u['total'];
      }
      else {
        $output .= '<td class="totals">'. $u['total'] .'</td><td class="gamesback">+'. $u['pts_back'] .'</td>';
      }
      foreach ($playoff_weeks as $wid => $week) {
        if ($u['weeks'][$wid]['total'] == $week_totals[$wid]["min"] && $week_totals[$wid]["min"] != $week_totals[$wid]["max"]) {
          $output .= '<td class="weekly weekly-low">'. $u['weeks'][$wid]['total'] .'</td>';
        }
        elseif ($u['weeks'][$wid]['total'] == $week_totals[$wid]["max"] && $week_totals[$wid]["min"] != $week_totals[$wid]["max"]) {
          $output .= '<td class="weekly weekly-high">'. $u['weeks'][$wid]['total'] .'</td>';
        }
        else {
          $output .= '<td>'. $u['weeks'][$wid]['total'] .'</td>';
        }
      }
      $output .= "</tr>";
    }
    $output .= '</table></fieldset></div>';
  }

  $output .= '</div>';
  print theme('page', $output);
}
