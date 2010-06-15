<?php
// $Id: $

/**
 * @file
 * Page callbacks for adding, editing, and deleting user picks.
 */


/**
 * Menu callback; presents the list of all weeks and games.
 */
function mypicks_all_page($league) {
  return theme('mypicks_all', $league);
}


/**
 * Display the list of all weeks and games.
 * The user would pick a week link from here to make their weekly picks.
 */
function theme_mypicks_all($league) {
  global $_PICKEM_MY_LEAGUES;
  global $user;

  // Param check: user must be a member of this league to continue.
  if ( !is_my_league($league) ) {
    return drupal_not_found();
  }

  drupal_set_title("My Picks - ". $user->name);

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
        $cols[] = array('data' => l($game->wk_number, "pickem/$league->lid/mypicks/" . $wid), 'class' => 'week');
      }
      $current_week .= '<div class="picks-float">' . $teams[$game->v_id]->short_name .'<br/>@'. $teams[$game->h_id]->short_name . '</div>';
    }
    $cols[] = $current_week;
    $rows[] = $cols;
  }
  return theme('table', $header, $rows, array('class' => 'pickem-picks'));
}


/**
 * Menu callback; presents a list of a single weeks picks.
 */
function mypicks_week_page($league, $week) {
  global $user;
  drupal_set_title($week->wk_name .' Picks - '. $user->name);
  return drupal_get_form('mypicks_week_form', $league, $week);
}


function mypicks_week_form(&$form_state=NULL, $league, $week) {
  global $user;

  $teams = get_teams(array('gameset_id' => $league->gsid));
  $games = get_games(array('week_id' => $week->wid));
  $picks = get_picks(array('league_id' => $league->lid, 'user_id' => $user->uid, 'week_id' => $week->wid));

  $form = array();
  $form['games'] = array('#tree' => TRUE);
  foreach ($games as $gid => $game) {

    $form['games'][$gid]['v_id'] = array(
      '#type' => 'hidden',
      '#value' => $game->v_id
    );
    $form['games'][$gid]['h_id'] = array(
      '#type' => 'hidden',
      '#value' => $game->h_id
    );

    if ($league->scoring_type == PICKEM_TYPE_WIN_LOSS || $league->scoring_type == PICKEM_TYPE_SPREAD) {

      if ($league->scoring_type == PICKEM_TYPE_WIN_LOSS) {
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
        '#default_value' => $picks[$gid][$user->uid]->winnerpick_id,
        '#attributes' => array('class' => 'pickem-team-radios'),
        '#options' => $options
      );
      
      // Disabled the radio boxes if the week has past.
      if ( $week->week_past == 1 ) {
      	$form['games'][$gid]['pick']['#attributes']['readonly'] = 'readonly';
      	$form['games'][$gid]['pick']['#attributes']['disabled'] = 'disabled';
      }
      
    }
    elseif ($league->scoring_type == PICKEM_TYPE_SCORE_DIFF) {
      $form['games'][$gid]['pick_v_score'] = array(
        '#type' => 'textfield',
        '#size' => 4,
        '#default_value' => $picks[$gid][$user->uid]->pick_v_score,
        '#field_suffix' => $teams[$game->v_id]->small_logo_img . $teams[$game->v_id]->short_name,
        '#prefix' => '<div class="pickem-team-score-diff">'
      );
      $form['games'][$gid]['pick_h_score'] = array(
        '#type' => 'textfield',
        '#size' => 4,
        '#default_value' => $picks[$gid][$user->uid]->pick_h_score,
        '#field_suffix' => $teams[$game->h_id]->small_logo_img . $teams[$game->h_id]->short_name,
        '#suffix' => '</div>'
      );
      
      // Disabled the input boxes if the week has past.
      if ( $week->week_past == 1 ) {
      	$form['games'][$gid]['pick_v_score']['#attributes']['readonly'] = 'readonly';
      	$form['games'][$gid]['pick_v_score']['#attributes']['disabled'] = 'disabled';
      	$form['games'][$gid]['pick_h_score']['#attributes']['readonly'] = 'readonly';
      	$form['games'][$gid]['pick_h_score']['#attributes']['disabled'] = 'disabled';
      }
      
      
    }
    $form['games'][$gid]['gametime'] = array(
      '#value' => $game->gametime
    );
  }

  $my_leagues = get_my_leagues($league->gsid);
  $my_list = array();
  foreach ($my_leagues as $l) {
    if ($l->scoring_type == $league->scoring_type) {
      $my_list[] = $l->name;
    }
  }

  $form['apply_to_all_my_leagues'] = array(
    '#type' => 'checkbox',
    '#access' => ($week->week_past == 0 && count($my_list) > 1),
    '#title' => t('Apply picks to all my leagues that use the same game set and scoring system.'),
    '#description' => implode(', ',$my_list),
    '#default_value' => 0
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#access' => ($week->week_past == 0),
    '#value' => t('Save Picks'),
    '#weight' => 5,
    '#submit' => array('mypicks_week_form_submit'),
  );

  return $form;
}


function theme_mypicks_week_form(&$form) {
  global $_PICKEM_TIMEZONE;

  $league = $form['#parameters'][2];

  $header = array();
  $header[] = t('Matchup');
  $header[] = t('Time(%pickem-timezone)', array('%pickem-timezone' => $_PICKEM_TIMEZONE));

  $rows = array();
  foreach (element_children($form['games']) as $gid) {
    $cols = array();

    if ($league->scoring_type == PICKEM_TYPE_WIN_LOSS || $league->scoring_type == PICKEM_TYPE_SPREAD) {
      $cols[] = drupal_render($form['games'][$gid]['pick']);
    }
    elseif ($league->scoring_type == PICKEM_TYPE_SCORE_DIFF) {
      $cols[] = drupal_render($form['games'][$gid]['pick_v_score']) . drupal_render($form['games'][$gid]['pick_h_score']);
    }

    $cols[] = drupal_render($form['games'][$gid]['gametime']);
    $rows[] = $cols;
  }

  $output = '';
  switch ($league->scoring_type) {
    case PICKEM_TYPE_WIN_LOSS:
      $output .= '<p>' . t('Just pick the winner.') . '</p>';
      break;
    case PICKEM_TYPE_SPREAD:
      $output .= '<p>' . t('Pick the winner taking the point spread into consideration.') . '</p>';
      break;
    case PICKEM_TYPE_SCORE_DIFF:
      $output .= '<p>' . t("Enter each team's final score.") . '</p>';
      break;
  }

  $output .= theme('table', $header, $rows);
  $output .= drupal_render($form);
  return $output;
}


function mypicks_week_form_submit($form, &$form_state) {
  global $user;

  $league = $form['#parameters'][2];
  $week = $form['#parameters'][3];

  // You should not be able to save picks after lockup.
  if ($week->week_past == 1) {
    drupal_set_title($week->wk_name .' picks are locked.');
    print theme('page', "You cannot change your picks now.");
    return;
  }

  // Check and see if the user wants to save to all their leagues that use the same gameset.
  $save_for_these_leagues = array();
  if ( $form_state['values']['apply_to_all_my_leagues'] == 1 ) {
    $save_for_these_leagues = get_my_leagues($league->gsid);
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
        set_pick($user->uid, $l, $pick);
      }
    }
  }


}
