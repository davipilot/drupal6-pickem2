<?php
// $Id: $

/**
 * @file
 * Page callbacks for the admin picks pages.
 */


function admin_picks_router($league = NULL, $account = NULL, $week = NULL) {
  if ( is_numeric($_POST['league_select']) ) {
    drupal_goto("admin/pickem/picks/" . $_POST['league_select']);
  }
  if ( is_numeric($_POST['user_select']) ) {
    drupal_goto("admin/pickem/picks/$league->lid/" . $_POST['user_select']);
  }

  if ( is_null($league) ) {
    drupal_set_title('Choose a League to administer');
    return drupal_get_form('league_select_form');
  }
  if ( is_null($account) ) {
    drupal_set_title('Choose a User to administer');
    return drupal_get_form('user_select_form', $league);
  }
}


/**
 * Menu callback; presents the list of all weeks and games.
 */
function admin_picks_all_page($league, $account) {
  return theme('admin_picks_all_page', $league, $account);
}


/**
 * Display the list of all weeks and games.
 * The user would pick a week link from here to make their weekly picks.
 */
function theme_admin_picks_all_page($league, $account) {

  // Param check: user must be a member of this league to continue.
  if ( !is_league_member($league, $account) ) {
    return drupal_not_found();
  }

  drupal_set_title("Administer Picks/Choose Week");

  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l($league->name, 'pickem/'. $league->lid);
  drupal_set_breadcrumb($breadcrumb);

  $teams = get_teams(array('gameset_id' => $league->gsid));
  $games = get_games(array('league_id' => $league->lid), array('by_week' => 1, 'order_by' => 'week_past, g.gametime, g.gid'));


  $header = array(t('Week'), t('Games'));
  $rows = array();
  foreach ($games as $wid => $weeks_games) {
    $cols = array();
    $current_week = '';
    foreach ($weeks_games as $gid => $game) {
      // The first column should be the week number link.
      if ( count($cols) == 0 ) {
        $cols[] = array('data' => l($game->wk_number, "admin/pickem/picks/$league->lid/$account->uid/" . $wid), 'class' => 'week');
      }
      $current_week .= '<div class="picks-float">' . $teams[$game->v_id]->short_name .'<br/>@'. $teams[$game->h_id]->short_name . '</div>';
    }
    $cols[] = $current_week;
    $rows[] = $cols;
  }

  $ouput = '';
  $output = 'League: '. $league->name .'<br/>';
  $output .= 'User: '. $account->name .'<br/>';
  $output .= theme('table', $header, $rows, array('class' => 'pickem-picks'));
  return $output;
}


/**
 * Menu callback; presents a list of a single weeks picks.
 */
function admin_picks_week_page($league, $week, $account) {
  drupal_set_title('Administer Picks');
  return drupal_get_form('admin_picks_week_form', $league, $week, $account);
}


function admin_picks_week_form(&$form_state=NULL, $league, $week, $account) {

  $teams = get_teams(array('gameset_id' => $league->gsid));
  $games = get_games(array('week_id' => $week->wid));
  $picks = get_picks(array('league_id' => $league->lid, 'user_id' => $account->uid, 'week_id' => $week->wid));

  $form = array();
  $form['games'] = array('#tree' => TRUE);
  $last = count($games);
  $count = 0;
  foreach ($games as $gid => $game) {

    $count++;
    $form['games'][$gid]['v_id'] = array(
      '#type' => 'hidden',
      '#value' => $game->v_id
    );
    $form['games'][$gid]['h_id'] = array(
      '#type' => 'hidden',
      '#value' => $game->h_id
    );

    if ($league->scoring_type == PICKEM_TYPE_WIN_LOSS || $league->scoring_type == PICKEM_TYPE_SPREAD || ($league->scoring_type == PICKEM_TYPE_WIN_LOSS_TIEBREAK && $count < $last)) {


      if ($league->scoring_type == PICKEM_TYPE_WIN_LOSS || $league->scoring_type == PICKEM_TYPE_WIN_LOSS_TIEBREAK) {
        $options =  array(
            $game->v_id => $teams[$game->v_id]->small_logo_img . $teams[$game->v_id]->short_name,
            $game->h_id => $teams[$game->h_id]->small_logo_img . $teams[$game->h_id]->short_name
        );
      }
      elseif ($league->scoring_type == PICKEM_TYPE_SPREAD) {
        $options =  array(
            $game->v_id => $teams[$game->v_id]->small_logo_img . $teams[$game->v_id]->short_name . '&nbsp;&nbsp;' . ($game->v_spread>0?'+'.$game->v_spread:$game->v_spread),
            $game->h_id => $teams[$game->h_id]->small_logo_img . $teams[$game->h_id]->short_name . '&nbsp;&nbsp;' . ($game->h_spread>0?'+'.$game->h_spread:$game->h_spread)
        );
      }

      $form['games'][$gid]['pick'] = array(
        '#type' => 'radios',
        '#default_value' => $picks[$gid][$account->uid]->winnerpick_id,
        '#attributes' => array('class' => 'pickem-team-radios'),
        '#options' => $options
      );

    }
    elseif ($league->scoring_type == PICKEM_TYPE_SCORE_DIFF || $league->scoring_type == PICKEM_TYPE_WIN_LOSS_TIEBREAK) {
      $form['games'][$gid]['pick_v_score'] = array(
        '#type' => 'textfield',
        '#size' => 4,
        '#default_value' => $picks[$gid][$account->uid]->pick_v_score,
        '#field_suffix' => $teams[$game->v_id]->small_logo_img . $teams[$game->v_id]->short_name,
        '#prefix' => '<div class="pickem-team-score-diff">'
      );
      $form['games'][$gid]['pick_h_score'] = array(
        '#type' => 'textfield',
        '#size' => 4,
        '#default_value' => $picks[$gid][$account->uid]->pick_h_score,
        '#field_suffix' => $teams[$game->h_id]->small_logo_img . $teams[$game->h_id]->short_name,
        '#suffix' => '</div>'
      );

    }
    $form['games'][$gid]['gametime'] = array(
      '#value' => $game->gametime
    );
  }

  $his_leagues = get_my_leagues($league->gsid, $account->uid);
  $his_list = array();
  foreach ($his_leagues as $l) {
    if ($l->scoring_type == $league->scoring_type) {
      $his_list[] = $l->name;
    }
  }

  $form['apply_to_all_my_leagues'] = array(
    '#type' => 'checkbox',
    '#access' => count($his_list) > 1,
    '#title' => t('Apply picks to all the user\'s leagues that use the same game set and scoring system.'),
    '#description' => implode(', ',$his_list),
    '#default_value' => 0
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Picks'),
    '#weight' => 5,
    '#submit' => array('admin_picks_week_form_submit'),
  );

  return $form;
}


function theme_admin_picks_week_form($form) {
  global $_PICKEM_TIMEZONE;

  $league = $form['#parameters'][2];
  $week = $form['#parameters'][3];
  $account = $form['#parameters'][4];

  $header = array();
  $header[] = t('Matchup');
  $header[] = t('Time(%pickem-timezone)', array('%pickem-timezone' => $_PICKEM_TIMEZONE));

  $rows = array();
  foreach (element_children($form['games']) as $gid) {
    $cols = array();

    if ($league->scoring_type == PICKEM_TYPE_WIN_LOSS || $league->scoring_type == PICKEM_TYPE_SPREAD) {
      $cols[] = drupal_render($form['games'][$gid]['pick']);
    }
    elseif ($league->scoring_type == PICKEM_TYPE_SCORE_DIFF || $league->scoring_type == PICKEM_TYPE_WIN_LOSS_TIEBREAK) {
      $cols[] = drupal_render($form['games'][$gid]['pick_v_score']) . drupal_render($form['games'][$gid]['pick_h_score']);
    }

    $cols[] = drupal_render($form['games'][$gid]['gametime']);
    $rows[] = $cols;
  }

  $output = '';
  $output .= 'League: '. $league->name .'<br/>';
  $output .= 'User: '. $account->name .'<br/>';
  $output .= 'Week: '. $week->wk_name .'<br/>';

  $output .= theme('table', $header, $rows);
  $output .= drupal_render($form);
  return $output;
}


function admin_picks_week_form_submit($form, &$form_state) {

  $league = $form['#parameters'][2];
  $week = $form['#parameters'][3];
  $account = $form['#parameters'][4];

  // Check and see if the user wants to save to all their leagues that use the same gameset.
  $save_for_these_leagues = array();
  if ( $form_state['values']['apply_to_all_my_leagues'] == 1 ) {
    $save_for_these_leagues = get_my_leagues($league->gsid, $account->uid);
  }
  else {
    $save_for_these_leagues[] = $league;
  }

  foreach ($save_for_these_leagues as $l) {
    // Only save for league using the same scoring system.
    if ($l->scoring_type == $league->scoring_type) {
      drupal_set_message("Saving picks for " . $l->name);
      foreach ($form_state['values']['games'] as $gid => $pick) {
        $pick['gid'] = $gid;
        set_pick($account->uid, $l, $pick);
      }
    }
  }


}
