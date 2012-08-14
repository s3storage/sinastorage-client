<?php
require('../SinaStorageService.php');

/**
 * Generated by PHPUnit_SkeletonGenerator on 2012-07-19 at 16:45:15.
 */
class SinaStorageServiceTest extends PHPUnit_Framework_TestCase
{
    /**
     * @var SinaStorageService
     */
    protected $object;
    protected $obj;

    public static function setUpBeforeClass()
    {

    }

    public static function tearDownAfterClass()
    {

    }

    /**
     * Sets up the fixture, for example, opens a network connection.
     * This method is called before a test is executed.
     */
    protected function setUp()
    {
        $conf = $this->getConf();
        $this->obj = SinaStorageService::getInstance($conf['project'],
                $conf['accesskey'], $conf['secretkey']);
        $this->obj->purgeParams();
        $this->obj->purgeReq();
        $this->obj->setAuth(true);
    }

    /**
     * Tears down the fixture, for example, closes a network connection.
     * This method is called after a test is executed.
     */
    protected function tearDown()
    {
        $this->obj->purgeParams();
        $this->obj->purgeReq();
    }

    /**
     * @covers SinaStorageService::uploadFile
     * @group uploadFile
     * @dataProvider uploadFileData
     */
    public function testUploadFile($localfile, $expected)
    {
        $httpCode = $this->upSingleFile($localfile);
        $this->assertEquals( $expected, $httpCode);
    }

    public function uploadFileData()
    {
        return array(
            array( 'file4UploadTest/pas.txt', 200),
            array( 'file4UploadTest/afine.txt', 200)
        );
    }

    /**
     * @covers SinaStorageService::uploadFileRelax
     * @group uploadFileRelax
     * @dataProvider uploadFileRelaxData
     */
    public function testUploadFileRelax($localfile, $expected)
    {
        $this->obj->setCURLOPTs(array(CURLOPT_VERBOSE=>0));
        $httpCode = $this->upSingleFile($localfile);

        if ($httpCode != 200) {
            $this->markTestSkipped("My debug message: Fail to upload $localfile");
        }

        $nameRelax = $localfile."Relax";
        $fileContent = "";
        $file_handle = fopen( $localfile , "r");
        while (!feof($file_handle)) {
            $fileContent = $fileContent.fread($file_handle,513);
        }
        fclose($file_handle);

        if ( $expected != 200) {
            $fileContent = "AddTimeInTheEnd--".date("U");
        }

        $file_sha1 = sha1($fileContent);
        $fileLength = strlen($fileContent);

        $this->obj->uploadFileRelax("$nameRelax",$file_sha1, $fileLength, $result);
        $httpCodeRelax = $this->getHttpCode($this->obj->result_info);

        $this->assertEquals( $expected, $httpCodeRelax);

    }

    public function uploadFileRelaxData()
    {
        return array(
            array( 'file4UploadTest/pas.txt', 200),
            array( 'file4UploadTest/afine.txt', 200),
            array( 'file4UploadTest/afine.txt', 400)
        );
    }

    /**
     * @covers SinaStorageService::copyFile
     * @group copyFile
     * @dataProvider copyFileData
     */
    public function testCopyFile($src,$expected)
    {
        $srcUpHttpCode = $this->upSingleFile($src);
        if ($srcUpHttpCode!=200) {
            $this->markTestSkipped("Fail to upload Source File to be copied.");
        }

        $dest = "copied/".array_pop(explode("/",$src,2));
        $this->obj->copyFile($dest, $src, $result);
        $httpCode = $this->getHttpCode($this->obj->result_info);
        $this->assertEquals($expected, $httpCode);
    }

    public function copyFileData()
    {
        return array(
            array( 'file4UploadTest/pas.txt', 200),
            array( 'file4UploadTest/afine.txt', 200)
        );
    }
    /**
     * @covers SinaStorageService::copyFileBetweenProject
     * @group copyFileBetweenProject
     * @dataProvider copyFileBetweenProjectData
     */
    public function testCopyFileBetweenProject($localfile, $expected)
    {
        $this->obj = SinaStorageService::getInstance
                ("yanhuihome","SYS0000000000SANDBOX","1111111111111111111111111111111111111111");
        $this->obj->setAuth(true);

        $httpCode = $this->upSingleFile($localfile);
        if ($httpCode!=200) {
            $this->markTestSkipped("Fail to upload Source File to be copied.");
        }

        $this->obj = SinaStorageService::getInstance
                ("sandbox2","SYS0000000000SANDBOX","1111111111111111111111111111111111111111");
        $this->obj->setAuth(true);
        $this->obj->copyFileBetweenProject($localfile, "yanhuihome", $localfile, $result);
        $copyResult = $this->getHttpCode($this->obj->result_info);

        $this->assertEquals($expected, $copyResult);
    }

    public function copyFileBetweenProjectData()
    {
        return array(
            array( 'file4UploadTest/pas.txt', 200),
            array( 'file4UploadTest/afine.txt', 200)
        );
    }
    /**
     * @covers SinaStorageService::getFile
     * @group getFile
     * @dataProvider getFileData
     */
    public function testGetFile( $filename, $ip, $expected)
    {
        if ( $ip ) {
            $this->obj->setExtra("?ip=$ip");
        }

        $this->obj->getFile( $filename, $result );
        $httpCode = $this->getHttpCode($this->obj->result_info);
        $this->assertEquals( $expected, $httpCode);
    }

    public function getFileData()
    {
        return array(
            array( '/file_uploaded/birth.txt', '', 200),
            array( '/file_uploaded/nnn3.txt', '', 200),
            array( 'foo/bar/1.html', '219.142.118.237', 200),
            array( 'foo/bar/1.html', '229.152.128.247', 403),
            array( 'nonexistfile.html', '', 404),
            array( 'foo/bar/1.html', '', 200)
       );
    }

    public function basicData()
    {
        return array(
            array( '/file_uploaded/birth.txt', 200),
            array( '/file_uploaded/nnn3.txt', 200),
            array( 'foo/bar/1.html', 200),
            array( 'nonexistfile.html', 404),
            array( 'foo/bar/1.html', 200)
       );
    }

    /**
     * @covers SinaStorageService::getFileUrl
     * @group getFileUrl
     * @dataProvider getFileUrlData
     */
    public function testGetFileUrl($file, $ip, $expected, $type)
    {
        if ( $ip ) {
            $this->obj->setExtra("?ip=$ip");
        }

        $this->obj->getFileUrl($file, $result);
        list($c_result,$c_info) = $this->myCURL($result, "GET", true);
        $this->assertEquals($expected, $c_info['http_code']);

        if ( $type == "DELETE" ) {
            $this->obj->getFileUrl($file, $delResult, "DELETE");
            list($c_result,$c_info) = $this->myCURL($delResult, "DELETE", true);
            $this->assertEquals( 204, $c_info['http_code']);

            $this->upSingleFile($file);
        }
   }

    public function getFileUrlData()
    {
        return array(
            array("foo/bar/1.html", "", 200, "GET"),
            array("foo/bar/1.html", "219.142.118.237", 200, "GET"),
            array("foo/bar/1.html", "229.152.128.247", 403, "GET"),
            array("nonexistfile.html", "", 404, "GET"),
            array("foo/bar/1.html", "", 200, "GET"),
            array("file4UploadTest/pas.txt", "", 200, "DELETE"),
        );
    }

    /**
     * @covers SinaStorageService::deleteFile
     * @group deleteFile
     * @dataProvider deleteFileData
     */
    public function testDeleteFile( $file, $expected)
    {
        $this->obj->deleteFile( $file, $result);
        $httpCode = $this->getHttpCode($this->obj->result_info);

        $this->assertEquals( $expected, $httpCode);
    }

    public function deleteFileData()
    {
        return array(
            array( 'file4UploadTest/pas.txt', 204),
            array( 'file4UploadTest/afine.txt', 204)
        );
    }

    /**
     * @covers SinaStorageService::getMeta
     * @group getMeta
     * @dataProvider basicData
     */
    public function testGetMeta($file, $expected)
    {
        $this->obj->getMeta($file,$result);
        $httpCode = $this->getHttpCode($this->obj->result_info);
        $this->assertEquals($expected,$httpCode);
    }

    /**
     * @covers SinaStorageService::updateMeta
     * @group updateMeta
     * @dataProvider basicData
     */
    public function testUpdateMeta($file,$expected)
    {
        $timeNow = date("U");
        $this->obj->setRequestHeaders(array("x-sina-info-int"=>$timeNow,"x-sina-info"=>"test_$file"));
        $this->obj->updateMeta($file, $result);
        $httpCode = $this->getHttpCode($this->obj->result_info);

        $this->assertEquals($expected, $httpCode);
    }

    /**
     * @covers SinaStorageService::getFileList
     * @group getFileList
     */
    public function testGetFileList()
    {
        $this->obj->getFileList($result);
        $httpCode = $this->getHttpCode($this->obj->result_info);
        $this->assertEquals(200,$httpCode);
    }

    /**
     * @covers SinaStorageService::listFiles
     * @group listFiles
     * @dataProvider listFilesData
     */
    public function testListFiles($marker, $pageeach, $prefix)
    {
        $result = $this->obj->listFiles($marker, $pageeach, $prefix);

        $this->assertEquals($marker, $result["Marker"]);
        $this->assertGreaterThan($marker, $result["Contents"][0]["Name"]);
        $this->assertEquals($pageeach, $result["ContentsQuantity"]);
    }

    public function listFilesData()
    {
        return array(
            array( 'foo/bar/1.html', 10, ""),
            array( 'file4UploadTest/afine.txt', 5, "IJustWriteSth")
        );
    }

    /**
     * @covers SinaStorageService::listProjectFiles
     * @group listProjectFiles
     * @dataProvider listProjectFilesData
     */
    public function testListProjectFiles($marker, $pageeach, $prefix, $expected)
    {
        $this->obj->listProjectFiles($marker, $pageeach, $prefix, $result);

        $httpCode = $this->getHttpCode($this->obj->result_info);
        $this->assertEquals($expected, $httpCode);

        $resultArr = json_decode($result, true);
        $this->assertEquals($marker, $resultArr["Marker"]);
        $this->assertGreaterThan($marker, $resultArr["Contents"][0]["Name"]);
        $this->assertEquals($pageeach, $resultArr["ContentsQuantity"]);
    }

    public function listProjectFilesData()
    {
        return array(
            array( 'foo/bar/1.html', 10, "",200),
            array( 'foo/bar/1.html', 10, "foo/bar",200),
            array( 'file4UploadTest/afine.txt', 5, "",200)
        );
    }

    /**
     * @covers SinaStorageService::setAuth
     * @group setAuth
     */
    public function testSetAuth()
    {
        $this->obj->setAuth();
        $this->obj->getFileUrl("foo/bar/1.html",$result);
        $uri = explode("?",$result,2);
        $this->assertNotEmpty($uri[1],"setAuth does not work.");

        $this->obj->setAuth(false);
        $this->obj->getFileUrl("foo/bar/1.html",$result);
        $uri = explode("?",$result,2);
        $this->assertEmpty($uri[1],"setAuth does not work.");

        $this->obj->setAuth(true);
        $this->obj->getFileUrl("foo/bar/1.html",$result);
        $uri = explode("?",$result,2);
        $this->assertNotEmpty($uri[1],"setAuth does not work.");
    }

    /**
     * @covers SinaStorageService::setExpires
     * @group setExpires
     */
    public function testSetExpires()
    {
        $this->obj->setAuth(false);
        $this->obj->getFileUrl("foo/bar/1.html",$result);

        $this->obj->setAuth(true);
        $this->obj->setExpires(121);
        $this->obj->getFileUrl("foo/bar/1.html",$result);

        $urlQueryStrArr = explode("&",$result);
        array_splice($urlQueryStrArr,0,1);

        $urlQueryStr = array();
        foreach ($urlQueryStrArr as $key) {
            $temp = explode("=", $key, 2);
            $urlQueryStr[$temp[0]] = $temp[1];
        }

        $this->assertArrayHasKey("ssig", $urlQueryStr, "setExpires fail.");
        $this->assertArrayHasKey("KID", $urlQueryStr, "setExpires fail.");
        $this->assertArrayHasKey("Expires", $urlQueryStr, "setExpires fail.");
    }

    /**
     * @covers SinaStorageService::setExtra
     * @group setExtra
     * @dataProvider setExtraData
     */
    public function testSetExtra($extra)
    {
        $this->obj->setAuth(false);
        $this->obj->setExtra();
        $this->obj->getFileUrl("foo/bar/1.html",$result);
        $urlParts = explode("?",$result, 2);
        $this->assertEquals( "", $urlParts[1]);

        $this->obj->setAuth(true);
        $this->obj->setExtra($extra);
        $this->obj->getFileUrl("foo/bar/1.html",$result);
        $urlParts = explode("?",$result, 2);
        $urlExtra = "?".substr($urlParts[1],0,strlen($extra)-1);
        $this->assertEquals($extra, $urlExtra);
    }

    public function setExtraData()
    {
        return array(
            array("?meta"),
            array("?acl"),
            array("?logging"),
            array("?relax"),
            array("?location")
        );
    }

    /**
     * @covers SinaStorageService::setQueryStrings
     * @group setQueryStrings
     * @dataProvider setQueryStringsData
     */
    public function testSetQueryStrings($queryStr)
    {
        $this->obj->setQueryStrings($queryStr);
        $this->obj->getFileUrl("foo/bar/1.html",$result);

        $urlQueryStrArr = explode("&",$result);
        array_splice($urlQueryStrArr,0,1);

        $urlQueryStr = array();
        foreach ($urlQueryStrArr as $key) {
            $temp = explode("=", $key, 2);
            $urlQueryStr[$temp[0]] = $temp[1];
        }

        $intersection = array_intersect($urlQueryStr,$queryStr);
        $this->assertEquals($queryStr, $intersection);
    }

    public function setQueryStringsData()
    {
        $qs1 = array(
            'formatter' => 'json',
            'marker' => 'makerVal',
            'max-keys' => 10
        );
        $qs2 = array(
            'doodle' => 'diagonal',
            'orientation' => 'heaven'
        );

        return array(
            array($qs1),
            array($qs2)
        );
    }

    /**
     * @covers SinaStorageService::setRequestHeaders
     * @group setRequestHeaders
     * @dataProvider setRequestHeadersData
     */
    public function testSetRequestHeaders($reqHeader)
    {
        $this->obj->setRequestHeaders($reqHeader);
        $reqHeaderGot = $this->obj->getRequestHeaders();

        $this->assertEquals($reqHeader, $reqHeaderGot);
    }

    public function setRequestHeadersData()
    {
        $reqHeader1 = array(
            "Content-Type" => "text/plain",
            "Content-Length" => "11",
            "Content-MD5" => "XrY7u+Ae7tCTyyK7j1rNww==",
        );
        $reqHeader2 = array(
            "Content-Type" => "text/plain",
            "s-sina-sha1" => "2aae6c35c94fcfb415dbe95f408b9ce91ee846ed",
            "s-sina-length" => "13",
        );
        return array(
            array($reqHeader1),
            array($reqHeader2)
        );
    }

    /**
     * @covers SinaStorageService::getRequestHeaders
     * @group getRequestHeaders
     * @dataProvider getRequestHeadersData
     */
    public function testGetRequestHeaders($reqHeader)
    {
        $reqHeaderGot = $this->obj->getRequestHeaders();
        $this->assertEmpty($reqHeaderGot);

        $this->obj->setRequestHeaders($reqHeader);
        $reqHeaderGot = $this->obj->getRequestHeaders();
        $this->assertEquals($reqHeader,$reqHeaderGot);
    }

    public function getRequestHeadersData()
    {
        $reqHeader1 = array(
            "Content-Type" => "text/plain",
            "Content-Length" => "11",
            "Content-MD5" => "XrY7u+Ae7tCTyyK7j1rNww==",
        );
        $reqHeader2 = array(
            "Content-Type" => "text/plain",
            "s-sina-sha1" => "2aae6c35c94fcfb415dbe95f408b9ce91ee846ed",
            "s-sina-length" => "13",
        );
        return array(
            array($reqHeader1),
            array($reqHeader2)
        );
    }

    /**
     * @covers SinaStorageService::setCURLOPTs
     * @group setCURLOPTs
     * @dataProvider setCURLOPTsData
     */
    public function testSetCURLOPTs($curlOpt)
    {
        $this->obj->setCURLOPTs();
        $result = $this->obj->getCURLOPTs();
        $this->assertEmpty($result);

        $this->obj->setCURLOPTs($curlOpt);
        $result = $this->obj->getCURLOPTs();

        if (!array_key_exists(CURLOPT_VERBOSE,$curlOpt)) {
            unset($result[CURLOPT_VERBOSE]);
        }
        $this->assertEquals($curlOpt, $result);
    }

    public function setCURLOPTsData()
    {
        $curlOpt1 = array(
            CURLOPT_HEADER=>1,
            CURLOPT_CONNECTTIMEOUT=>10,
            CURLOPT_RETURNTRANSFER=>1
        );
        $curlOpt2 = array(
            CURLOPT_VERBOSE=>True,
            CURLOPT_TIMEOUT=>60
        );

        return array(
            array($curlOpt1),
            array($curlOpt2)
        );
    }

    /**
     * @covers SinaStorageService::setTimeout
     * @group setTimeout
     * @dataProvider setTimeoutData
     */
    public function testSetTimeout($timeout)
    {
        $this->obj->setDomain("http://gz.sinastorage.com/");
        $this->obj->setTimeout($timeout);
        $curlOpts = $this->obj->getCURLOPTs();

        $this->assertEquals( $timeout, $curlOpts[CURLOPT_TIMEOUT]);
    }

    public function setTimeoutData()
    {
        return array(
            array( 3 ),
            array( 7 )
        );
    }

    /**
     * @covers SinaStorageService::setDomain
     * @group setDomain
     */
    public function testSetDomain()
    {
        $this->obj->getFileUrl("foo/bar/1.html",$result);
        $this->assertStringStartsWith("http://sinastorage.com/", $result);

        $this->obj->setDomain("http://IJustChangeTheDomain.com/");

        $this->obj->getFileUrl("foo/bar/1.html",$result);
        $this->assertStringStartsWith("http://IJustChangeTheDomain.com/", $result);
    }

    /**
     * @covers SinaStorageService::getCURLOPTs
     * @group getCURLOPTs
     * @dataProvider getCURLOPTsData
     */
    public function testGetCURLOPTs($curlOpt)
    {
        $this->obj->setCURLOPTs($curlOpt);
        $result = $this->obj->getCURLOPTs();

        if (!array_key_exists(CURLOPT_VERBOSE,$curlOpt)) {
            unset($result[CURLOPT_VERBOSE]);
        }
        $this->assertEquals($curlOpt, $result);
    }

    public function getCURLOPTsData()
    {
        $curlOpt1 = array(
            CURLOPT_HEADER=>1,
            CURLOPT_CONNECTTIMEOUT=>10,
            CURLOPT_RETURNTRANSFER=>1
        );
        $curlOpt2 = array(
            CURLOPT_VERBOSE=>True,
            CURLOPT_TIMEOUT=>60
        );

        return array(
            array($curlOpt1),
            array($curlOpt2)
        );
    }
    /**
     * @covers SinaStorageService::purgeParams
     * @group purgeParams
     * @dataProvider purgeParamsData
     */
    public function testPurgeParams($queryStr, $curlOpt, $requestHeader)
    {
        $this->obj->setExpires(123);
        $this->obj->setExtra("?acl");
        $this->obj->setQueryStrings($queryStr);
        $this->obj->setCURLOPTs($curlOpt);
        $this->obj->setRequestHeaders($requestHeader);

        $this->obj->getFileUrl("foo/bar/1.html",$result);
        $uri = explode("?",$result,2);
        $getCurlOpt = $this->obj->getCURLOPTs();
        $this->assertNotEmpty($uri[1],"Fail to setExtra and setQueryStrings.");
        $this->assertNotEmpty($getCurlOpt,"Fail to setCURLOPTs.");

        $this->obj->purgeParams();

        $this->obj->getFileUrl("foo/bar/1.html",$result);
        $uri = explode("?",$result,2);
        $getCurlOpt = $this->obj->getCURLOPTs();
        $this->assertEmpty($uri[1],"Fail to purgeParams.");
        $this->assertEmpty($getCurlOpt,"Fail to purgeParams.");
    }

    public function purgeParamsData()
    {
        $queryStr = array(
            'formatter' => 'json',
            'marker' => 'makerVal',
            'max-keys' => 10
        );
        $curlOpt = array(
            CURLOPT_HEADER=>1,
            CURLOPT_CONNECTTIMEOUT=>10,
            CURLOPT_RETURNTRANSFER=>1
        );
        $requestHeader = array(
            "Content-Type" => "text/plain",
            "Content-Length" => "11",
            "Content-MD5" => "XrY7u+Ae7tCTyyK7j1rNww=="
        );
        return array(
            array($queryStr, $curlOpt, $requestHeader)
        );
    }

    /**
     * @covers SinaStorageService::purgeReq
     * @group purgeReq
     * @dataProvider purgeReqData
     */
    public function testPurgeReq($queryStr, $curlOpt, $requestHeader)
    {
        $this->obj->setExtra("?acl");
        $this->obj->setQueryStrings($queryStr);
        $this->obj->setCURLOPTs($curlOpt);
        $this->obj->setRequestHeaders($requestHeader);

        $this->obj->getFileUrl("foo/bar/1.html",$result);
        $uri = explode("?",$result,2);
        $getCurlOpt = $this->obj->getCURLOPTs();
        $this->assertNotEmpty($uri[1],"Fail to setExtra and setQueryStrings.");
        $this->assertNotEmpty($getCurlOpt,"Fail to setCURLOPTs.");

        $this->obj->purgeReq();
        $this->obj->setAuth(false);

        $this->obj->getFileUrl("foo/bar/1.html",$result);
        $uri = explode("?",$result,2);
        $getCurlOpt = $this->obj->getCURLOPTs();
        $this->assertEmpty($uri[1],"Fail to purgeReq.");
        $this->assertEmpty($getCurlOpt,"Fail to purgeReq.");
    }

    public function purgeReqData()
    {
        $queryStr = array(
            'formatter' => 'json',
            'marker' => 'makerVal',
            'max-keys' => 10
        );
        $curlOpt = array(
            CURLOPT_HEADER=>1,
            CURLOPT_CONNECTTIMEOUT=>10,
            CURLOPT_RETURNTRANSFER=>1
        );
        $requestHeader = array(
            "Content-Type" => "text/plain",
            "Content-Length" => "11",
            "Content-MD5" => "XrY7u+Ae7tCTyyK7j1rNww=="
        );
        return array(
            array($queryStr, $curlOpt, $requestHeader)
        );
    }

    public function getConf()
    {
        $conf = array('project'=>'sandbox', 'accesskey'=>'SYS0000000000SANDBOX',
                 'secretkey'=>'1111111111111111111111111111111111111111');
        return $conf;
    }

    public function myCURL($url, $type, $header=true)
    {
        $c = curl_init();
        curl_setopt($c, CURLOPT_URL, $url);
        curl_setopt($c, CURLOPT_CUSTOMREQUEST, $type);
        curl_setopt($c, CURLOPT_HEADER, $header);
        curl_setopt($c, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($c, CURLOPT_TIMEOUT, 3);

        $c_result = curl_exec($c);
        $c_info = curl_getinfo($c);
        curl_close($c);

        return array($c_result, $c_info);
    }

    public function upSingleFile($localfile)
    {
        $fileType = mime_content_type( $localfile );
        $fileLength = filesize($localfile);
        $fileName = $localfile;
        $fileContent = "";

        $file_handle = fopen( $localfile , "r");
        while (!feof($file_handle)) {
            $fileContent = $fileContent.fread($file_handle,513);
        }
        fclose($file_handle);

        $file_sha1 = sha1($fileContent);
        $this->obj->uploadFile("$fileName",$fileContent, $fileLength, $fileType, $result);
        $httpCode = $this->getHttpCode($this->obj->result_info);

        return $httpCode;
    }

    public function getHttpCode($result_info)
    {
        return $result_info['http_code'];
    }

}
?>
