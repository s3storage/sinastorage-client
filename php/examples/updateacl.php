<?php
require("../SinaStorageService.php");
require_once( "conf.php" );

$o = SinaStorageService::getInstance($project, $accesskey, $secretkey);

//可自定义配置的CURLOPT
$o->setCURLOPTs(array(CURLOPT_VERBOSE=>1));

//设置使用验证方式
$o->setAuth(true);

var_dump($o->updateACL("2.html",array( "SINA00000000000000XP"=>array( "read", "write" ) ), $result));
echo $result;

var_dump($o->getACL("2.html", $result));
echo $result;
