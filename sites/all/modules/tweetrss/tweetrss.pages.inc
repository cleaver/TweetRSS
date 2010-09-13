<?php
// $Id$

/**
 * @file
 * Administration screens for tweetrss module.
 *
 * Created on Apr 6, 2010
 */

/**
 * Build feed list form
 */
function tweetrss_admin_form() {

  $sql = "SELECT r.tfid, r.twitter_uid, a.screen_name, r.fid, f.title, r.feedtype
      FROM {tweetrss_feed} r,
        {twitter_account} a,
        {aggregator_feed} f
      WHERE r.twitter_uid = a.twitter_uid
        AND r.fid = f.fid
      ORDER BY a.screen_name";
  $results = db_query($sql);
  while ($row = db_fetch_array($results)) {
    $rows[] = array(
      $row['screen_name'],
      $row['title'],
      $row['feedtype'],
      l(t('edit'), "admin/settings/tweetrss/edit/{$row['tfid']}"),
    );
  }
  $header = array(
    t('Twitter account'),
    t('RSS feed'),
    t('Feed type'),
    t('Operations'));
  $output = theme('table', $header, $rows);

  return $output;
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


/**
 * Default theme function for add/edit form
 *
 */
function theme_tweetrss_add_form($form) {
  $output = drupal_render($form);
  return $output;
}


/**
 * Save feed information to database
 *
 */
function tweetrss_feed_save($feed) {

  if ($feed['tfid'] == 0) {
    // insert new record
    if ($feed['twitter_uid'] > 0 && $feed['fid'] > 0) {
      $record = (object) NULL;
      $record->twitter_uid    = $feed['twitter_uid'];
      $record->fid            = $feed['fid'];
      $record->feedtype       = $feed['feedtype'];
      $record->regex_match    = $feed['regex_match'];
      $record->regex_replace  = $feed['regex_replace'];
      $record->checked        = $feed['checked'];
      drupal_write_record('tweetrss_feed', $record);
    }
    else {
      drupal_set_message(t('Twitter account and Feed must have values'), 'error');
    }
  }
  else {
      $sql =
        "UPDATE {tweetrss_feed} SET
          twitter_uid = %d,
          fid = %d,
          feedtype = '%s',
          regex_match = '%s',
          regex_replace = '%s',
          checked =%d
        WHERE tfid = %d";
      db_query($sql,
        $feed['twitter_uid'],
        $feed['fid'],
        $feed['feedtype'],
        $feed['regex_match'],
        $feed['regex_replace'],
        $feed['checked'],
        $feed['tfid']);
  }
}


/**
 * Retrieve a feed row by primary key.
 *
 */
function tweetrss_feed_read($tfid) {
    $sql =
      'SELECT
          twitter_uid,
          fid,
          feedtype,
          regex_match,
          regex_replace,
          checked
      FROM {tweetrss_feed}
      WHERE tfid = %d';
  $result = db_query($sql, $tfid);
  $feed = db_fetch_array($result);
  return $feed;
}


/**
 * Delete a feed row by primary key.
 *
 */
function tweetrss_feed_delete($tfid) {
  $sql =
      'DELETE FROM {tweetrss_feed}
      WHERE tfid = %d';
  $result = db_query($sql, $tfid);
}


/**
 * Build feed add / edit form
 *
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
  $form['feedtype'] = array(
    '#type' => 'select',
    '#title' => t('RSS feed'),
    '#options' => tweetrss_module_list(),
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
    $form['feedtype']['#default_value'] = $feed['feedtype'];
    $form['regex_match']['#default_value'] = tweetrss_filter_xss($feed['regex_match']);
    $form['regex_replace']['#default_value'] = tweetrss_filter_xss($feed['regex_replace']);
    $form['checked']['#default_value'] = $feed['checked'];
  }

  return $form;
}

/**
 * Handle submitted feed add/edit form.
 *
 */
function tweetrss_add_form_submit($form, &$form_state) {
  switch ($form_state['values']['op']) {
    case t('Save feed'):
      tweetrss_feed_save($form_state['values']);
      break;
    case t('Delete'):
      tweetrss_feed_delete($form_state['values']['tfid']);
      break;
    case t('Cancel'):
      break;
  }
  $form_state['redirect'] = 'admin/settings/tweetrss';
}

/**
 * Filter out possible XSS vulnerabilities.  Since it is applied
 * to the regex strings, it has to be more permissive than check_plain(),
 * filter_xss(), etc.  This function is just a hacked version of D6
 * version of filter_xss().
 *
 */
function tweetrss_filter_xss($string) {
  // Only operate on valid UTF-8 strings. This is necessary to prevent cross
  // site scripting issues on Internet Explorer 6.
  if (!drupal_validate_utf8($string)) {
    return '';
  }
  // Remove Netscape 4 JS entities
  $string = preg_replace('%&\s*\{[^}]*(\}\s*;?|$)%', '', $string);

  // Defuse all HTML entities
  $string = str_replace('&', '&amp;', $string);
  // Change back only well-formed entities in our whitelist
  // Decimal numeric entities
  $string = preg_replace('/&amp;#([0-9]+;)/', '&#\1', $string);
  // Hexadecimal numeric entities
  $string = preg_replace('/&amp;#[Xx]0*((?:[0-9A-Fa-f]{2})+;)/', '&#x\1', $string);
  // Named entities
  $string = preg_replace('/&amp;([A-Za-z][A-Za-z0-9]*;)/', '&\1', $string);

  return $string;
}

