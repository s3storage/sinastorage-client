<?php
require("../SinaStorageService.php");

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

//设置使用验证方式
$o->setAuth(true);
//$o->setExtra("?formatter=xml");

echo "========getFileList=========\n";
var_dump($o->getFileList($result));
echo $result;
echo "========getFileList=========\n";
echo "\n\n";

