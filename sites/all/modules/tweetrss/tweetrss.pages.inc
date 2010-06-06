<?php
/*
 * Created on Apr 6, 2010
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */
/**
 * Build feed list form
 */
function tweetrss_admin_form($form_state) {
watchdog('tweetrss', 'admin form');

  $form['#tree'] = TRUE;
  $form['feeds'] = array();
  $sql = "SELECT r.tfid, r.twitter_uid, a.screen_name, r.fid, f.title, r.checked, r.feedurl
      FROM {tweetrss_feed} r,
        {twitter_account} a,
        {aggregator_feed} f
      WHERE r.twitter_uid = a.twitter_uid
        AND r.fid = f.fid
      ORDER BY a.screen_name";
  $results = db_query($sql);
  while ($row = db_fetch_array($results)) {
    $form['feeds'][] = _tweetrss_feed_row($row);
  }
//  $form['feeds'][] = _tweetrss_feed_row(array());

  dsm($form);
  return $form;
}


function _tweetrss_feed_row($feed) {
  $form['#feed'] = $feed;
  $form['screen_name'] = array(
    '#value' => $feed['screen_name']
  );
  $form['title'] = array(
    '#value' => $feed['title']
  );
  $form['edit_link'] = array(
    '#value' => l('Edit', "admin/settings/tweetrss/edit/{$feed['tfid']}"),
  );
//  $form['checked'] = array(
//    '#type' => 'textfield',
//    '#default_value' => $feed['checked'],
//    '#size' => 9,
//    '#maxlength' => 9
//  );
//  $form['feedurl'] = array(
//    '#type' => 'textfield',
//    '#default_value' => $feed['feedurl'],
//    '#size' => 50,
//    '#maxlength' => 255
//  );
//  if (empty($feed)) {
//    // new record
//  $form['new'] = array(
//    '#type' => 'value',
//    '#value' => TRUE
//  );
//  $form['twitter_uid'] = array(
//    '#type' => 'select',
//    '#options' => _get_account_list()
//  );
//  $form['fid'] = array(
//    '#type' => 'select',
//    '#options' => _get_feed_list()
//  );
//  }
//  else {
//    // existing record
//  $form['new'] = array(
//    '#type' => 'value',
//    '#value' => FALSE
//  );
//  $form['twitter_uid'] = array(
//    '#type' => 'value',
//    '#value' => $feed['twitter_uid']
//  );
//  $form['fid'] = array(
//    '#type' => 'value',
//    '#value' => $feed['fid']
//  );
//  $form['delete'] = array(
//      '#type' => 'checkbox'
//    );
//  }
  return $form;
}

function _tweetrss_get_feed_list() {
  $list = array(-1 => '---');
  $sql = "SELECT fid, title FROM {aggregator_feed}";
  $results = db_query($sql);
  while ($row = db_fetch_array($results)) {
    $list[$row['fid']] = $row['title'];
  }
  return $list;
}

function _tweetrss_get_account_list() {
  $list = array(-1 => '---');
  $sql = "SELECT twitter_uid, screen_name FROM {twitter_account}";
  $results = db_query($sql);
  while ($row = db_fetch_array($results)) {
    $list[$row['twitter_uid']] = $row['screen_name'];
  }
  return $list;
}

function theme_tweetrss_admin_form($form) {
//return drupal_render($form);

  $header = array(
    t('Twitter Account'),
    t('RSS Feed'),
    t('Last Feed Item'),
    t('Feed URL'),
    t('Delete'));
  $rows = array();

  foreach (element_children($form['feeds']) as $key) {
    $element = &$form['feeds'][$key];
    $row = array(
      drupal_render($element['screen_name']),
      drupal_render($element['title']),
      drupal_render($element['edit_link']),
    );
    $rows[] = $row;
  }
  $output = theme('table', $header, $rows);
  $output .= drupal_render($form['buttons']);
  return $output;
}

function theme_tweetrss_add_form($form) {
  $output = drupal_render($form);
  return $output;
}

/**
 * Handle submitted tweetrss admin Form
 */
function tweetrss_admin_form_submit($form, &$form_state) {
  $feeds = $form_state['values']['feeds'];
  foreach($feeds as $feed) {
    if (empty($feed['delete'])) {
      tweetrss_feed_save($feed);
    }
    else {
      tweetrss_feed_delete($feed);
    }
  }
}

/**
 * Save feed information to database
 */
function tweetrss_feed_save($feed) {

  if ($feed['tfid'] == 0) {
    // insert new record
    if ($feed['twitter_uid'] > 0 && $feed['fid'] > 0) {
      $sql =
        "INSERT INTO {tweetrss_feed} (
          twitter_uid,
          fid,
          regex_match,
          regex_replace,
          checked )
        VALUES (%d, %d, '%s', '%s', %d)";
      db_query($sql,
        $feed['twitter_uid'],
        $feed['fid'],
        $feed['regex_match'],
        $feed['regex_replace'],
        $feed['checked']);
    }
    else {
      drupal_set_message('Twitter account and Feed must have values', 'error');
    }
  }
  else {
      $sql =
        "UPDATE {tweetrss_feed} SET
          twitter_uid = %d,
          fid = %d,
          regex_match = '%s',
          regex_replace = '%s',
          checked =%d
        WHERE tfid = %d";
      db_query($sql,
        $feed['twitter_uid'],
        $feed['fid'],
        $feed['regex_match'],
        $feed['regex_replace'],
        $feed['checked'],
        $feed['tfid']);
  }
}

/**
 * Retrieve a feed row by primary key.
 */
function tweetrss_feed_read($tfid) {
    $sql =
      'SELECT
          twitter_uid,
          fid,
          regex_match,
          regex_replace,
          checked
      FROM {tweetrss_feed}
      WHERE tfid = %d';
  $result = db_query($sql, $tfid);
  $feed = db_fetch_array($result);
  return $feed;
}
function tweetrss_feed_delete($feed) {
  // delete the record
  $sql = "DELETE FROM {tweetrss_feed}" .
      " WHERE twitter_uid = %d" .
      " AND fid = %d";
  db_query($sql, $feed['twitter_uid'], $feed['fid']);
}

/**
 * Build feed add / edit form
 */
function tweetrss_add_form($form_state, $tfid=0) {

  $form['tfid'] = array(
    '#type' => 'value',
    '#value' => $tfid,
  );
  $form['twitter_uid'] = array(
    '#type' => 'select',
    '#title' => t('Twitter account'),
    '#options' => _tweetrss_get_account_list(),
  );
  $form['fid'] = array(
    '#type' => 'select',
    '#title' => t('RSS feed'),
    '#options' => _tweetrss_get_feed_list(),
  );
  $form['regex_match'] = array(
    '#type' => 'textfield',
    '#title' => t('Regex match'),
    '#size' => 70,
  );
  $form['regex_replace'] = array(
    '#type' => 'textfield',
    '#title' => t('Regex replace'),
    '#size' => 70,
  );
  $form['checked'] = array(
    '#type' => 'textfield',
    '#title' => t('Last imported'),
//    '#default_value' => $feed['checked'],
    '#size' => 10,
    '#maxlength' => 10,
  );
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save feed'),
  );
  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );
  $form['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
  );

  //retrieve existing record if exists
  if ($tfid > 0) {
    $feed = tweetrss_feed_read($tfid);
    $form['twitter_uid']['#default_value'] = $feed['twitter_uid'];
    $form['fid']['#default_value'] = $feed['fid'];
    $form['regex_match']['#default_value'] = $feed['regex_match'];
    $form['regex_replace']['#default_value'] = $feed['regex_replace'];
    $form['checked']['#default_value'] = $feed['checked'];
  }

  return $form;
}

/**
 * Handle submitted feed add form.
 */
function tweetrss_add_form_submit($form, &$form_state) {
  switch ($form_state['values']['op']) {
    case t('Save feed'):
      dsm('save');
      tweetrss_feed_save($form_state['values']);
    break;
    case t('Delete'):
      dsm('delete');
    break;
    case t('Cancel'):
      dsm('cancel');
    break;
  }
  $form_state['#redirect'] = 'admin/settings/tweetrss';
  dsm($form_state);
}

