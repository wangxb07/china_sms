<?php

$plugin = array(
  'name' => 'C123',
  'module' => 'china_sms',
  'send' => 'china_sms_c123_send',
  'result_parse' => 'china_sms_result_parse',
  'settings form' => 'china_sms_c123_settings_form',
);

/**
 * send message to target phone number
 * @param $message
 * @param $to
 * @return
 *
 */
function china_sms_c123_send($conf, $message, $to) {
  $url = $conf->data['url'];
  $ac = $conf->data['app_id'];
  $authkey = $conf->data['app_secret'];
  $cgid = $conf->data['cgid'];
  $m = $to;
  $c = $message;
  $csid = '';
  $t = '';
  return _china_sms_c123_send_sms($url, $ac, $authkey, $cgid, $m, $c, $csid, $t);
}

function china_sms_result_parse($received_data) {

}

function _china_sms_c123_send_sms($url, $ac, $authkey, $cgid, $m, $c, $csid, $t) {
  $data = array(
    'action'	=> 'sendOnce', // 发送类型 ，可以有sendOnce短信发送，sendBatch一对一发送，sendParam动态参数短信接口
    'ac'		=> $ac, // 用户账号
    'authkey'	=> $authkey, // 认证密钥
    'cgid'		=> $cgid, // 通道组编号
    'm'			=> $m, // 号码,多个号码用逗号隔开
    'c'			=> $c, // 内容
    'csid'		=> $csid, // 签名编号 ，可以为空，为空时使用系统默认的签名编号
    't'			=> $t // 定时发送，为空时表示立即发送
  );

  $xml = _china_sms_c123_post_sms($url, $data); // POST方式提交
  $re = simplexml_load_string(utf8_encode($xml));

  if(trim($re['result']) == 1) { // 发送成功 ，返回企业编号，员工编号，发送编号，短信条数，单价，余额
    foreach ($re->Item as $item) {
      $stat['msgid'] = trim((string)$item['msgid']);
      $stat['total'] = trim((string)$item['total']);
      $stat['price'] = trim((string)$item['price']);
      $stat['remain'] = trim((string)$item['remain']);
      $stat_arr[] = $stat;
    }
    if(is_array($stat_arr)) {
      // watchdog('china_sms', "send sms success by c123 plugin, response body
      // !response_body", array('!response_body' => (string)$re['result']),
      // WATCHDOG_INFO);
      return TRUE;
    }
  }
  else { // 发送失败的返回值
    $error = _china_sms_c123_message_mapping((string)$re['result']);
    watchdog('china_sms', "send sms failed by c123 plugin, error message !message", array('!message' => $error), WATCHDOG_ERROR);
    return FALSE;
  }
}

function _china_sms_c123_message_mapping($message) {
  switch($message) {
    case  0:
      return "帐户格式不正确(正确的格式为:员工编号@企业编号)";
      break;
    case  -1:
      return "服务器拒绝(速度过快、限时或绑定IP不对等)如遇速度过快可延时再发";
      break;
    case  -2:
      return " 密钥不正确";
      break;
    case  -3:
      return "密钥已锁定";
      break;
    case  -4:
      return "参数不正确(内容和号码不能为空，手机号码数过多，发送时间错误等)";
      break;
    case  -5:
      return "无此帐户";
      break;
    case  -6:
      return "帐户已锁定或已过期";
      break;
    case  -7:
      return "帐户未开启接口发送";
      break;
    case  -8:
      return "不可使用该通道组";
      break;
    case  -9:
      return "帐户余额不足";
      break;
    case  -10:
      return "内部错误";
      break;
    case  -11:
      return "扣费失败";
      break;
  }
}

function _china_sms_c123_post_sms($url, $data = '') {
  $row = parse_url($url);
  $host = $row['host'];
  $port = isset($row['port']) ? $row['port']:80;
  $file = $row['path'];

  $post = '';
  while (list($k,$v) = each($data)) {
    $post .= rawurlencode($k)."=".rawurlencode($v)."&";	//转URL标准码
  }

  $post = substr($post , 0 , -1);
  $len = strlen($post);
  $fp = @fsockopen($host, $port, $errno, $errstr, 10);

  if (!$fp) {
    return "$errstr ($errno)\n";
  }
  else {
    $receive = '';
    $out = "POST $file HTTP/1.0\r\n";
    $out .= "Host: $host\r\n";
    $out .= "Content-type: application/x-www-form-urlencoded\r\n";
    $out .= "Connection: Close\r\n";
    $out .= "Content-Length: $len\r\n\r\n";
    $out .= $post;
    fwrite($fp, $out);
    while (!feof($fp)) {
      $receive .= fgets($fp, 128);
    }
    fclose($fp);
    $receive = explode("\r\n\r\n",$receive);
    unset($receive[0]);
    return implode("",$receive);
  }
}

/**
 * config form callback
 */
function china_sms_c123_settings_form($conf) {
  $conf->data = $conf->data + array(
    'url' => '',
    'app_id' => '',
    'app_secret' => '',
    'cgid' => '',
  );

  $form['url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL'),
    '#default_value' => $conf->data['url'],
    '#description' => t('C123 API URL'),
  );

  $form['app_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Application ID'),
    '#default_value' => $conf->data['app_id'],
    '#description' => t('Application ID for C123 API'),
  );

  $form['app_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Secret'),
    '#default_value' => $conf->data['app_secret'],
    '#description' => t('Application Secret'),
  );

  $form['cgid'] = array(
    '#type' => 'textfield',
    '#title' => t('CGID'),
    '#default_value' => $conf->data['cgid'],
    '#description' => t('Channel group id'),
  );
  return $form;
}
