<?php
require("../SinaStorageService.php");
require_once( "conf.php" );

$file_content = "123";
$file_length = 3;
$file_type = 'text/html';
$file_sha1 = sha1($file_content);

$o = SinaStorageService::getInstance($project, $accesskey, $secretkey);

//可自定义配置的CURLOPT
$o->setCURLOPTs(array(CURLOPT_VERBOSE=>1));
$o->setQueryStrings(array("sldkfj"=>123));

//设置使用验证方式
$o->setAuth(true);

var_dump($o->uploadFile("foo/bar/myfn",$file_content, $file_length, $file_type, $result));
var_dump( $result );
