<?php
// $Id: $

/**
 * @file
 * Page callbacks for the main league dashboard.
 */


/**
 * Menu callback; presents the list of all weeks and games.
 */
function league_page($league) {
  return theme('league_page', $league);
}

function theme_league_page($league) {
  drupal_set_title($league->name .' League');

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[1] = l($league->name, 'pickem/'. $league->lid);
  drupal_set_breadcrumb($breadcrumb);

  $o  = '<div class="pickem"><table class="league-nav"><tr>';
  $o .= '<td class="col1">'. theme('league_nav', $league) .'</td>';
  if ( is_my_league($league) ) {
    $o .= '<td class="col2">'. theme('my_league_summary', $league) .'</td>';
  }
  else {
    $o .= '<td class="col2">&nbsp;</td>';
  }
  $o .= '</tr>';
  $o .= '<tr><td colspan="2">'. theme('league_messages', $league) .'</td></tr>';
  $o .= '</table></div>';
  return $o;
}


function theme_league_nav($league) {
    global $user;

    $o = '';
    $o .=  '<ul>';

    if ( is_my_league($league) ) {
      $o .= '<li>'. l('My Picks', 'pickem/'. $league->lid .'/mypicks') .'</li>';
    }

    $cur_w = get_current_week($league, FALSE);
    if ($cur_w->wk_number <> '') {
      $o .= '<li>'. l('Pick Summary('. $cur_w->wk_number .')', 'pickem/'. $league->lid .'/weeklypicks/'. $cur_w->wid) .'</li>';
    }
    else {
      //print_r($cur_w);
    }

    $o .= '<li>'. l('Standings', 'pickem/'. $league->lid .'/standings') .'</li>';

    if ( $league->rules_node_id != 0 ) {
      $o .= '<li>'. l('Rules and Regs', 'node/'. $league->rules_node_id) .'</li>';
    }

    if ( $league->forum_id != 0 ) {
      $o .= '<li>'. l('Talk and Taunting', 'forum/'. $league->forum_id) .'</li>';
    }

    if ( user_access('administer pickem') && $cur_w->wk_number <> '' ) {
      $o .= '<li>'. l('Update Scores', 'admin/pickem/scores/'. $league->lid . '/edit/' . $cur_w->wid) .'</li>';
    }

    $o .= '<li>Research<ul>';
    if ( $league->partial_league == 0 ) {
      $o .= '<li>'. l('Schedule Grid', 'pickem/'. $league->lid .'/grid') .'</li>';
    }
    if (module_exists("open_flash_chart_api")) {
      $o .= '<li>'. l('High/Low Chart', 'pickem/'. $league->lid .'/chart1') .'</li>';
      if ( $league->partial_league == 0 ) {
        $o .= '<li>'. l('Team Pickability Chart', 'pickem/'. $league->lid .'/chart2') .'</li>';
      }
    }
    if ( $league->partial_league == 0 ) {
      $o .= '<li>'. l('Team Win-Loss Stats', 'pickem/'. $league->lid .'/team_stats') .' / '. l('vPower', 'pickem/'. $league->lid .'/team_power') .'</li>';
    }

    $o .= '</ul></li></ul>';

    return theme('dash', 'Navigation', $o);
}


function theme_my_league_summary($league) {
  //drupal_set_message('theme_my_league_summary');
  global $user;

  $weeks = get_weeks(array('gameset_id' => $league->gsid));

  $totals = get_user_totals($league); //sorted by overall score
  $title = 'My Summary';
  $content = '<div class="my-league-summary">';
  $content .= 'RANK: #'. $totals[$user->uid]['ranking'] .'<br/>';
  if ( $totals[$user->uid]['ranking'] > 1 ) {
    $content .= 'POINTS BACK: '. $totals[$user->uid]['pts_back'] .'<br/>';
  }
  $content .= 'TOTAL POINTS: '. $totals[$user->uid]['total'] .'<br/>';
  if ( $totals[$user->uid]['best_week_id'] <> -1 ) {
    $content .= 'BEST WEEK SCORE:  '. $totals[$user->uid]['weeks'][$totals[$user->uid]['best_week_id']]['total'] .' ('. $weeks[$totals[$user->uid]['best_week_id']]->wk_name .')<br/>';
    $content .= 'WORST WEEK SCORE:  '. $totals[$user->uid]['weeks'][$totals[$user->uid]['worst_week_id']]['total'] .' ('. $weeks[$totals[$user->uid]['worst_week_id']]->wk_name .')<br/>';
  }
  $content .= '</div>';
  return theme('dash', $title, $content);
}


function theme_league_messages($league) {
  global $user;
  global $_PICKEM_TIMEZONE;

  if ( is_my_league($league) && $league->forum_id != 0 ) {
    $post_link = '/node/add/forum/'. $league->forum_id;
    $output = '<a href="'. $post_link .'">Post new forum topic.</a>';
  }
  $output .= '<ul>';
  $cur_w = get_current_week($league, TRUE);

  if ( $cur_w->firstgame_date != '' ) {
    if ( $cur_w->firstgame_dow <> "Sunday" ) {
      $output .= '<li class="warning">Warning: The first game is on <span style="text-decoration:underline;">'. strtoupper($cur_w->firstgame_dow) .'</span>.</li>';
    }
    if (module_exists("jstimer")) {
      $output .= '<li class="warning">'. get_countdown_timer($cur_w) .'</span></li>';
    }
    else {
      $output .= '<li class="warning">Week '. ($cur_w->wk_number) .' picks due by '. date('l, F j, Y \a\t g:i a', strtotime($cur_w->firstgame)) .'('. $_PICKEM_TIMEZONE .").</li>";
    }
  }


  if ( is_my_league($league) && $league->forum_id != 0 ) {
    $sql = db_rewrite_sql("SELECT n.nid, n.title, l.comment_count, l.last_comment_timestamp, u.name, c.cid, c.comment, c.subject comment_subject, cu.name comment_name, c.timestamp comment_timestamp FROM {node} n INNER JOIN {node_comment_statistics} l ON n.nid = l.nid inner join {users} u on u.uid=n.uid inner join {term_node} tn on tn.nid=n.nid left outer join {comments} c on c.nid=n.nid left outer join {users} cu on cu.uid=c.uid WHERE n.status = 1 AND n.type = 'forum' and tn.tid=%d ORDER BY GREATEST(n.created, IF(c.timestamp IS NULL, 0, c.timestamp)) DESC");
    $result = db_query($sql, $league->forum_id);

    $i = 1;
    $content='';
    while ($node = db_fetch_object($result)) {
      if ( $i > 4 ) {
        break;
      }
      $i++;
      if ( $node->comment ) {
        $content .= '<li>'. l($node->comment_subject .' - '. $node->comment_name .' <Re: '. $node->title .'>', 'node/'. $node->nid  )  .  ' ('. format_date($node->comment_timestamp, 'small') .') <a href="/comment/reply/'. $node->nid .'/'. $node->cid .'">-Reply</a></li>';
      }
      else {
        $content .= '<li>'. l($node->title .' - '. $node->name, 'node/'. $node->nid  )  .' ('. format_date($node->last_comment_timestamp, 'small') .') <a href="/comment/reply/'. $node->nid .'/#comment-form">-Reply</a></li>';
      }
    }

    if ( $content == '' ) {
      $content = "<li>None</li>";
    }

    $output .= $content;
  }
  $output .= '</ul>';

  return theme('dash', 'Messages', $output);
}


function theme_dash($title, $content, $region='main', $link=NULL) {
  if ( isset($link) ) {
    $output = '<div class="dash"><div class="title">'. $link .'</div><div>'. $content .'</div></div>';
  }
  else {
    $output = '<div class="dash"><div class="title">'. $title .'</div><div>'. $content .'</div></div>';
  }
  return $output;
}
