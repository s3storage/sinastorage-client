<?php
require("../SinaStorageService.php");
require_once( "conf.php" );

// sandbox2 has ssk conf
$project = "sandbox2";

$file_content = "123";
$file_length = 3;
$file_type = 'text/html';

$o = SinaStorageService::getInstance($project, $accesskey, $secretkey);

/*
 * //可自定义配置的CURLOPT
 * $o->setCURLOPTs(array(CURLOPT_VERBOSE=>1));
 * // casual query string for tracing/debug
 * $o->setQueryStrings(array("sldkfj"=>123));
 */

//设置使用验证方式
$o->setAuth(true);

// ssk/ is required, foo/ is optional
var_dump($o->uploadFile("ssk/foo/",$file_content, $file_length, $file_type, $result));

$download_url = $o->resp[ 'headers' ][ 'x-sina-serverside-key' ];
echo "curl $project.sinastorage.com/$download_url\n";
