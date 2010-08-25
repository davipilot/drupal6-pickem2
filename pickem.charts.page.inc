<?php
// $Id: $

/**
 * @file
 * Page callbacks for display of various charts using open_flash_chart_api.
 */


function chart_hi_low_page($league, $account = NULL) {
  drupal_set_title('Charts');

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l($league->name, 'pickem/'. $league->lid);
  drupal_set_breadcrumb($breadcrumb);

  $g = new open_flash_chart_api();

  if ( $account === NULL ) {
    $g->set_title( t('Weekly Points High/Low Chart'), '{font-size: 20px;}' );
  }
  else {
    $g->set_title( $account->name .': '. t('Weekly Points High/Low Chart'), '{font-size: 20px;}' );
  }

  $g->set_width('100%');
  $g->set_height('480');

  $weeks = get_totals_by_week($league);

  // League AVG line.
  $b = array();
  $y_max = 0;
  foreach ($weeks as $w) {
    if ( $w['count'] == '' ) {
      $b[] = 'null';
    }
    else {
      $b[] = $w['total'] / $w['count'];
    }
    if ( $w['max'] > $y_max ) {
      $y_max = $w['max'];
    }
  }
  $y_max = ceil($y_max/5)*5;

  $g->set_data($b);
  $g->line_dot( 2, 4, '#008000', 'League AVG', 12 );

  // User line, if needed.
  if ( $account !== NULL ) {
    $ut = get_user_totals($league);
    $c = array();
    foreach ($weeks as $wid => $w) {
      if ( $ut[$account->uid]['weeks'][$wid]['total'] == '' ) {
        $c[] = 'null';
      }
      else {
        $c[] = $ut[$account->uid]['weeks'][$wid]['total'];
      }
    }
    $g->set_data($c);
    $g->line_hollow( 2, 5, '#000080', $ut[$account->uid]['name'], 12 );
  }

  // Candle box for min-max.
  $a = array();
  foreach ($weeks as $w) {
    if ( $w['max'] == '' ) {
      $a[] = new candle('null', 'null', 'null', 'null');
    }
    else {
      $a[] = new candle($w['max'], $w['min'], $w['max'], $w['min']);
    }
  }
  $g->candle( $a, 70, 1, '#800000', 'High/Low', 12 );


  // X axis:
  $x_labels = array();
  $weeks = get_weeks(array('gameset_id' => $league->gsid));
  foreach ($weeks as $wid => $week) {
    $x_labels[] = $week->wk_number;
  }
  $g->set_x_labels( $x_labels );
  $g->set_x_legend( 'Week', 12);

  if ( isset($y_max) ) {
    $g->set_y_max($y_max);
  }
  $g->y_label_steps( 5 );
  $g->set_y_legend( 'Points', 12);

  $g->set_bg_colour('0xDFFFDF');

  $g->set_tool_tip( '#x_legend# #x_label# (#val#)' );

  $o = '<div class="pickem">'. $g->render('js');

  $users = get_users($league, 0, variable_get('pickem_full_user_load', 0));
  foreach ($users as $u) {
    $o .= '<span class="user-list-horiz">';
    $name = $u->name;
    $name = str_replace(' ', '&nbsp;', $name);
    if ( $account->uid == $u->uid ) {
      $o .= $name;
    }
    else {
      // Problem here for RealName.  This is not a normal /user/uid link.
      $o .= l($name, 'pickem/'. $league->lid .'/chart1/'. $u->uid, array('html' => TRUE));
    }
    $o .= '</span>';
  }
  $o .= '</div>';
  print theme('page', $o);

}


function chart_team_pickability_page($league, $sort_by='win_lose') {
  drupal_set_title('Charts');

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l($league->name, 'pickem/'. $league->lid);
  drupal_set_breadcrumb($breadcrumb);

  $g = new open_flash_chart_api();
  $g->set_title( t('Team Pickability (Regular Season)'), '{font-size: 20px;}' );
  $g->set_width('100%');
  $g->set_height('480');

  $teams = get_team_totals($league, $sort_by);
  $x_labels = array();
  $a = array();
  foreach ($teams as $tid => $team_data) {
    $a[] = is_null($team_data['pickability'])?'NULL':$team_data['pickability'];
    $x_labels[] = $team_data['abbrev'];
  }
  $g->set_data($a);
  $g->area_hollow( 2, 3, 25, '#000080', 'Correctly Picked', 12);

  $a = array();
  foreach ($teams as $tid => $team_data) {
    $a[] = is_null($team_data['pickability_w'])?'NULL':$team_data['pickability_w'];
  }
  $g->set_data($a);
  $g->area_hollow( 2, 3, 25, '#800080', 'Correctly Picked To Win', 12);


  $g->set_x_labels( $x_labels );
  $g->set_x_label_style( 12, '#000000', 2);

  $g->set_x_legend( 'Percent that a team is correctly picked', 12);

  $g->set_y_max( 100 );
  $g->y_label_steps( 10 );
  $g->set_y_legend( 'Percent', 12);

  $g->set_bg_colour('0xDFFFDF');
  //echo $g->render();

  $g->set_tool_tip( '#x_label# (#val#)' );

  $o = $g->render('js');

  if ($sort_by == 'to_win' ) {
    $o .= l('Sort By "Correctly Picked"', 'pickem/'. $league->lid .'/chart2/win_lose', array('html' => TRUE));
  }
  else {
    $o .= l('Sort By "Correctly Picked To Win"', 'pickem/'. $league->lid .'/chart2/to_win', array('html' => TRUE));
  }

  print theme('page', $o);
}


function chart_team_scoring_page($league, $team_id) {
  drupal_set_title('Charts');

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l($league->name, 'pickem/'. $league->lid);
  drupal_set_breadcrumb($breadcrumb);

  $teams = get_teams(array('gameset_id' => $league->gsid));

  $g = new open_flash_chart_api();
  $g->set_title( t('Team Scoring - '. $teams[$team_id]->tm_abbrev ), '{font-size: 20px;}' );
  $g->set_width('100%');
  $g->set_height('480');

  $x_labels = array();
  $pf = array();
  $pa = array();
  $games = get_games(array('gameset_id' => $league->gsid, 'team_id' => $team_id));
  foreach ($games as $gid => $row) {
    if ( is_null($row->h_score) || is_null($row->v_score)  ) {
      continue;
    }

    if ( $team_id == $row->h_id ) {
      $pf[] = $row->h_score;
      $pa[] = $row->v_score;
      $x_labels[] = $teams[$row->v_id]->tm_abbrev;
    }
    else {
      $pf[] = $row->v_score;
      $pa[] = $row->h_score;
      $x_labels[] = $teams[$row->h_id]->tm_abbrev;
    }
  }
  $g->set_data($pf);
  $g->area_hollow( 2, 3, 25, '#000080', 'Points For', 12);

  $g->set_data($pa);
  $g->area_hollow( 2, 3, 25, '#800080', 'Points Against', 12);

  $g->set_x_labels( $x_labels );
  $g->set_x_label_style( 12, '#000000', 2);

  $g->set_x_legend( 'Opponent', 12);

  $g->set_y_max( 60 );
  $g->y_label_steps( 10 );
  $g->set_y_legend( 'Points', 12);

  $g->set_bg_colour('0xDFFFDF');
  //echo $g->render();

  $g->set_tool_tip( $teams[$team_id]->tm_abbrev .' vs #x_label# (#val#)');

  $o = $g->render('js');

  print theme('page', $o);
}
