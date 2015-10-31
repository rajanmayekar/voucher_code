<?php

/**
 * @file
 * Admin specific functions, pages for voucher code.
 */

function voucher_code_admin_list() {
  $voucher_codes = _voucher_code_get_all();
  $rows = array();

  foreach ($voucher_codes as $voucher_code) {
    $row = array();

    $row['code'] = $voucher_code->code;
    $row['active_from'] = $voucher_code->active_from ? format_date($voucher_code->active_from) : '-';
    $row['active_to'] = $voucher_code->active_to ? format_date($voucher_code->active_to) : '-';
    $row['limit_count'] = $voucher_code->limit_count ? $voucher_code->limit_count : t('Unlimited');
    $row['is_active'] = $voucher_code->is_active ? t('Yes') : t('No');
    $row['action'] = theme('links', array(
      'links' => array(
        array('title' => t('Edit'), 'href' => 'admin/config/people/voucher_code/edit/' . $voucher_code->code_id),
        array('title' => t('Delete'), 'href' => 'admin/config/people/voucher_code/delete/' . $voucher_code->code_id),
      ),
      'attributes' => array('class' => array('links', 'inline')),
    ));

    $rows[] = $row;
  }

  return theme('table', array(
      'header' => array(
        'code' => t('Code'),
        'active_from' => t('Active from'),
        'active_to' => t('Active till'),
        'limit_count' => t('Limit'),
        'is_active' => t('Active?'),
        'action' => '#',
      ),
      'rows' => $rows,
  ));
}

function voucher_code_admin_create($form, $form_state, $code_id = null) {
  $form = array();
  $voucher = NULL;

  $active_from = $active_to = array('day' => 0, 'month' => 0, 'year' => 0);

  if (is_numeric($code_id)) {
    $voucher = _voucher_code_get_by_id($code_id);

    if ($voucher->active_from) {
      $date_obj = new DateTime();
      $date_obj->setTimestamp($voucher->active_from);
      $active_from['day'] = $date_obj->format('j');
      $active_from['month'] = $date_obj->format('n');
      $active_from['year'] = $date_obj->format('Y');
    }

    if ($voucher->active_to) {
      $date_obj = new DateTime();
      $date_obj->setTimestamp($voucher->active_to);
      $active_to['day'] = $date_obj->format('j');
      $active_to['month'] = $date_obj->format('n');
      $active_to['year'] = $date_obj->format('Y');
    }
  }

  $form['voucher_code'] = array(
    '#type' => 'fieldset',
    '#title' => $voucher ? t('Update voucher code') : t('Create voucher code'),
    '#tree' => TRUE,
  );

  $form['voucher_code']['code_id'] = array(
    '#type'          => 'value',
    '#value'         => $voucher,
  );

  $form['voucher_code']['code'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Voucher code'),
    '#required'      => TRUE,
    '#description'   => t('Must be unique'),
    '#default_value' => $voucher ? $voucher->code: '',
  );

  $form['voucher_code']['active_from'] = array(
    '#type'          => 'date',
    '#title'         => t('Active from'),
    '#description'   => t('When this code should activate (leave blank to activate immediately). Accepts any date format that strtotime can handle.'),
    '#default_value' => $active_from,
    '#after_build'   => array('_voucher_code_date_optional'),
    '#element_validate' => array('_voucher_code_date_validate'),
  );

  $form['voucher_code']['active_to'] = array(
    '#type'          => 'date',
    '#title'         => t('Active till'),
    '#description'   => t('When this code should activate (leave blank to activate immediately). Accepts any date format that strtotime can handle.'),
    '#default_value' => $active_to,
    '#after_build'   => array('_voucher_code_date_optional'),
    '#element_validate' => array('_voucher_code_date_validate'),
  );

  $form['voucher_code']['limit_count'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Maximum uses'),
    '#default_value' => $voucher ? $voucher->limit_count : 1,
    '#size'          => 10,
    '#required'      => TRUE,
    '#description'   => t('How many times this code can be used (enter 0 for unlimited).'),
  );


  $form['voucher_code']['is_active'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Make it active?'),
    '#default_value' => $voucher ? $voucher->is_active : TRUE,
  );

  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => $voucher ? t('Update') : t('Create'),
  );

  return $form;
}

function voucher_code_admin_create_validate($form, $form_state) {
  $voucher = NULL;
  $voucher_values = $form_state['values']['voucher_code'];
  $existing = voucher_code_get_by_code($voucher_values['code']);

  if ($existing) {
    if (!empty($voucher_values['code_id'])) {
      if ($existing->code_id !== $voucher_values['code_id']->code_id) {
        form_set_error('voucher_code][code', t('This code is already exist.'));
      }
    } else {
      form_set_error('voucher_code][code', t('This code is already exist.'));
    }
  }
}

function voucher_code_admin_create_submit($form, $form_state) {
  $voucher = $form_state['values']['voucher_code'];

  if ($voucher['active_from']['year']) {
    $voucher['active_from'] = mktime(0, 0, 0, $voucher['active_from']['month'], $voucher['active_from']['day'], $voucher['active_from']['year']);
  } else {
    $voucher['active_from'] = 0;
  }

  if ($voucher['active_to']['year']) {
    $voucher['active_to'] = mktime(0, 0, 0, $voucher['active_to']['month'], $voucher['active_to']['day'], $voucher['active_to']['year']);
  } else {
    $voucher['active_to'] = 0;
  }

  $voucher['is_active'] = $voucher['is_active'] ? 1 : 0;

  $code_id = !empty($voucher['code_id']) ? $voucher['code_id']->code_id : NULL;

  unset($voucher['code_id']);

  _voucher_code_create($voucher, $code_id);

  if ($code_id) {
    drupal_set_message(t('Voucher code %code is updated successfully.', array('%code' => $voucher['code'])));
  } else {
    drupal_set_message(t('Voucher code %code is created successfully.', array('%code' => $voucher['code'])));
  }
}

function voucher_code_admin_delete($form, $form_state, $code_id) {
  $voucher = _voucher_code_get_by_id($code_id);

  $form = array();

	$form['code_id'] = array(
		'#type' => 'value',
		'#value' => $voucher,
	);

  return confirm_form(
		$form,
		t('Are you sure you want to delete voucher code %code ?', array('%code' => $voucher->code)),
		'admin/config/people/voucher_code',
		t('This action cannot be undone.'),
		t('Delete'),
		t('Cancel')
	);
}

function voucher_code_admin_delete_submit($form, $form_state) {
  if ($form_state['values']['code_id']) {
    _voucher_code_delete_by_id($form_state['values']['code_id']->code_id);

    drupal_set_message(t('Voucher code %code is updated successfully.', array('%code' => $form_state['values']['code_id']->code)));
  }
  drupal_goto('admin/config/people/voucher_code');
}

function _voucher_code_delete_by_id($code_id) {
  db_delete('voucher_code')
    ->condition('code_id', $code_id)
    ->execute();
}

function _voucher_code_get_all() {
  $query = db_select('voucher_code', 'vc')
    ->fields('vc');

  $result = $query->execute();

  return $result->fetchAll();
}

function _voucher_code_get_by_id($code_id) {
  $query = db_select('voucher_code', 'vc')
    ->fields('vc')
    ->condition('vc.code_id', $code_id);

  $result = $query->execute();

  return $result->fetch();
}

function _voucher_code_create($values, $code_id = NULL) {
  if ($code_id) {
    db_update('voucher_code')
      ->fields($values)
      ->condition('code_id', $code_id)
      ->execute();
  } else {
    db_insert('voucher_code')
      ->fields($values)
      ->execute();
  }
}

/**
 * Make a date field optional.
 */
function _voucher_code_date_optional($element, &$form_state) {
  foreach (array('year', 'month', 'day') as $field) {
    $element[$field]['#options'][0] = '--';
    ksort($element[$field]['#options']);
  }

  return $element;
}


/**
 * Ensure a blank date validates.
 */
function _voucher_code_date_validate($element) {
  $blank = TRUE;
  foreach (array('year', 'month', 'day') as $field) {
    $blank = $blank && $element[$field]['#value'][0] === '0';
  }
  if ($blank) {
    return TRUE;
  }
  return date_validate($element);
}
