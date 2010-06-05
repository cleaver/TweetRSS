<?php
/*
 * Created on Apr 6, 2010
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
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
    '#value' => $feed['tfid'],
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

function _get_feed_list() {
  $list = array(-1 => '---');
  $sql = "SELECT fid, title FROM {aggregator_feed}";
  $results = db_query($sql);
  while ($row = db_fetch_array($results)) {
    $list[$row['fid']] = $row['title'];
  }
  return $list;
}

function _get_account_list() {
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

function tweetrss_feed_save($feed) {
dsm($feed);

  if ($feed['new'] == TRUE) {
    // insert new record
    if ($feed['twitter_uid'] > 0 && $feed['fid'] > 0) {
      $sql = "INSERT INTO {tweetrss_feed}" .
        " (twitter_uid, fid, feedurl, checked) " .
        " VALUES (%d, %d, '%s', %d)";
      db_query($sql, $feed['twitter_uid'], $feed['fid'], $feed['feedurl'], $feed['checked']);
    }
  }
  else {
    // update existing record
  }
}

function tweetrss_feed_delete($feed) {
  // delete the record
  $sql = "DELETE FROM {tweetrss_feed}" .
      " WHERE twitter_uid = %d" .
      " AND fid = %d";
  db_query($sql, $feed['twitter_uid'], $feed['fid']);
}
