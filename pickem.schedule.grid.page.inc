<?php
// $Id: $

/**
 * @file
 * Page callbacks and theme functions for the schedule grid page.
 */


function schedule_grid_page($league) {
  drupal_set_title("Schedule Grid");

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l($league->name, 'pickem/'. $league->lid);
  drupal_set_breadcrumb($breadcrumb);

  $output = '';
  $cache_key = 'pickem_page_grid_'. $league->gsid;
  if ( ($cache = cache_get($cache_key)) && !empty($cache->data) ) {
    drupal_add_js('misc/tableheader.js');
    $output = $cache->data;
  }
  else {
    $weeks = get_weeks(array('league_id' => $league->lid, 'playoffs' => 0));

    $header = array('&nbsp;');
    foreach ($weeks as $wid => $week) {
      $header[] = array('data' => $week->wk_number, 'title' => 'Week Number '. $week->wk_number);
    }

    $query = "SELECT g.gid, g.wid, h.tm_abbrev as home, v.tm_abbrev as visitor, g.h_score, g.v_score FROM {pickem_games} g inner join {pickem_teams} h on h.tid=g.h_id inner join {pickem_teams} v on v.tid=g.v_id order by wid";
    $result = db_query($query);

    $games = array();
    while ($row = db_fetch_object($result)) {
      if ($row->h_score > $row->v_score) {
        // Home Victory
        $games[$row->home][$row->wid] = array(
          "op" => $row->visitor,
          "w-l" => "win",
          "h-a" => "home",
          "score" => $row->h_score .'-'. $row->v_score
        );
        $games[$row->visitor][$row->wid] = array(
          "op" => $row->home,
          "w-l" => "loss",
          "h-a" => "away",
          "score" => $row->v_score .'-'. $row->h_score
        );
      }
      elseif ($row->v_score > $row->h_score) {
        // Visitor Victory
        $games[$row->home][$row->wid] = array(
          "op" => $row->visitor,
          "w-l" => "loss",
          "h-a" => "home",
          "score" => $row->h_score .'-'. $row->v_score
        );
        $games[$row->visitor][$row->wid] = array(
          "op" => $row->home,
          "w-l" => "win",
          "h-a" => "away",
          "score" => $row->v_score .'-'. $row->h_score
        );
      }
      elseif ($row->v_score == $row->h_score && $row->h_score <> "") {
        // TIE
        $games[$row->home][$row->wid] = array(
          "op" => $row->visitor,
          "w-l" => "tie",
          "h-a" => "home",
          "score" => $row->h_score .'-'. $row->v_score
        );
        $games[$row->visitor][$row->wid] = array(
          "op" => $row->home,
          "w-l" => "tie",
          "h-a" => "away",
          "score" => $row->v_score .'-'. $row->h_score
        );
      }
      else {
        // TBD, game not played
        $games[$row->home][$row->wid] = array(
          "op" => $row->visitor,
          "w-l" => "tbd",
          "h-a" => "home",
          "score" => 'N/A'
        );
        $games[$row->visitor][$row->wid] = array(
          "op" => $row->home,
          "w-l" => "tbd",
          "h-a" => "away",
          "score" => 'N/A'
        );
      }
    }
    ksort($games);


    $output = '<div class="pickem">Hover over completed game for the score.';

    $rows = array();
    foreach ($games as $tm => $teamsgames) {
      ksort($teamsgames);
      $row = array();
      $row[] = array('data' => $tm, 'class' => 'team_head');
      foreach ($weeks as $w => $w_data) {
        if (isset ($teamsgames[$w])) {
          $class .= $teamsgames[$w]["h-a"];
          $class .= " ". $teamsgames[$w]["w-l"];
          if ($teamsgames[$w]["h-a"] == "away") {
            $teamsgames[$w]["op"] = '@'. $teamsgames[$w]["op"];
          }
          $row[] = array('data' => $teamsgames[$w]["op"], 'class' => $teamsgames[$w]["w-l"], 'title' => strtoupper($teamsgames[$w]["w-l"]) .' '. $teamsgames[$w]["score"]);
        }
        else {
          if ($w_data->playoffs == 1) {
            // playoffs, blank, used for teams that didn't make the playoffs
            $row[] = '&nbsp;';
          }
          else {
            // bye week
            $row[] = array('data' => 'bye', 'class' => 'bye', 'title' => 'Bye Week');
          }
        }
      }
      $rows[] = $row;
    }

    $output .= theme('table', $header, $rows, array('class' => 'grid'));
    $output .= '</div>';

    cache_set($cache_key, $output);
  }

  return $output;
}