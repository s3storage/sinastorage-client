<?php
require("SinaService/SinaStorageService/SinaStorageService.php");

$project = "sandbox";
$accesskey = "SYS0000000000SANDBOX";
$secretkey = "1111111111111111111111111111111111111111";

//写一个测试文件
$localfile = "1.html";
$content = <<<HTML
<html>
  <body>
    <p>hello world.</p>
  </body>
</html>
HTML;
file_put_contents($localfile, $content);

//准备请求需要的数据
$file_content = file_get_contents($localfile);
$file_length = filesize($localfile);
$file_sha1 = sha1($file_content);

$o = SinaStorageService::getInstance($project, $accesskey, $secretkey);

//可自定义配置的CURLOPT
$o->setCURLOPTs(array(CURLOPT_VERBOSE=>1));
$o->getFileUrl("1123.html",$result);
echo $result,"\n";

//设置使用验证方式
$o->setAuth(true);

//设置IP访问权限,获取一个只允许以下IP访问的url
$o->setExtra("?ip=61.135.152.194");
//设置下载用户的ip地址被限制在61.135.152.0/24网段，并在UNIX时间1175135000后生效
$o->setExtra("?ip=1175135000,61.135.152.");

$o->getFileUrl("1123.html",$result);
echo $result;
