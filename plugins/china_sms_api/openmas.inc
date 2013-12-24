<?php

$plugin = array(
  'name' => 'openmas',
  'module' => 'china_sms',
  'send' => 'china_sms_openmas_send',
  'settings form' => 'china_sms_openmas_settings_form',
);

/**
 * send message to target phone number
 * @param $message
 * @param $to
 * @return
 *
 */
function china_sms_openmas_send($conf, $message, $to) {
  $library = libraries_load('nusoap');

  if (empty($library['loaded'])) {
    watchdog('china_sms', "Couldn't find the NuSOAP library. send message aborted.", array(), WATCHDOG_ERROR);

    return FALSE;
  }

  $client = new nusoap_client(_china_sms_openmas_get_data($conf, 'url') ,true);
  $client->soap_defencoding = 'utf-8';
  $client->decode_utf8 = false;
  $client->xml_encoding = 'utf-8';
  $err = $client->getError();
  if ($err) {
    watchdog('china_sms', '<h2>连接webservice出错</h2><pre>@error</pre>', array('@error' => $err), WATCHDOG_ERROR);
  }

  if (!is_array($to)) {
    $to = array($to);
  }

  $paras = array(
    'destinationAddresses' => $to,
    'message' => $message,
    'extendCode' => _china_sms_openmas_get_data($conf, 'extend_code'),
    'applicationId' => _china_sms_openmas_get_data($conf, 'app_id'),
    'password' => _china_sms_openmas_get_data($conf, 'app_secret'),
    'expectSendTime' => 0,
  );

  try {
    //第一个参数是方法名，第二个参数是数组(所有的方法的参数都放到数组中)
    if ($paras['expectSendTime']) {
      $result = $client->call('SendMessage4', $paras);
    }
    else {
      $result = $client->call('SendMessage3', $paras);
    }

    if ($client->fault) {
      watchdog('china_sms', '<h2>Client Fualt</h2><pre>@error</pre>', array('@error' => var_export($result, TRUE)), WATCHDOG_ERROR);
    } else {
      $err = $client->getError();
      if ($err) {
        watchdog('china_sms', '<h2>Fualt by get error</h2><pre>@error</pre>', array('@error' => $err), WATCHDOG_ERROR);
      } else {
        return $result;
      }
    }
  }
  catch ( SoapFault $e ) {
    watchdog('china_sms', '<h2>Fualt throw by Soap</h2><pre>@error</pre>', array('@error' => $e->getMessage ()), WATCHDOG_ERROR);
  }
  catch ( Exception $e ) {
    watchdog('china_sms', '<h2>Fualt by exception</h2><pre>@error</pre>', array('@error' => $e->getMessage ()), WATCHDOG_ERROR);
  }
}

function _china_sms_openmas_get_data($conf, $name, $default = '') {
  return isset($conf->data[$name]) ? $conf->data[$name] : $default;
}

/**
 * config form for openmas
 */
function china_sms_openmas_settings_form($conf) {

  $form['url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL'),
    '#default_value' => _china_sms_openmas_get_data($conf, 'url'),
    '#description' => t('OpenMAS webservice URL'),
  );

  $form['app_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Application ID'),
    '#default_value' => _china_sms_openmas_get_data($conf, 'app_id'),
    '#description' => t('Application ID for OpenMAS webservice'),
  );

  $form['app_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Secret'),
    '#default_value' => _china_sms_openmas_get_data($conf, 'app_secret'),
    '#description' => t('Application Secret'),
  );

  $form['extend_code'] = array(
    '#type' => 'textfield',
    '#title' => t('Extend Code'),
    '#default_value' => _china_sms_openmas_get_data($conf, 'extend_code'),
    '#description' => t('OpenMAS extend code for outer application'),
  );

  return $form;
}
