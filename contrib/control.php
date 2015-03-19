<?php

$obj = new S3ClFS_Control('/tmp/s3clfs-s3clfs.sock');

var_dump($obj->getInfo());
$obj->ping();
$obj->loop();

class S3ClFS_Control {
	private $fp;
	private $info;
	private $id;

	public function __construct($path) {
		$this->fp = fsockopen('unix://'.$path, -1);
		if (!$this->fp) throw new Exception('Failed to connect');
		$this->info = $this->readPacket();
		$this->id = 0;
	}

	public function getInfo() {
		return $this->info;
	}

	public function ping() {
		$now = (int)(microtime(true)*1000000);
		$this->sendPacket(['command' => 'ping', 'ts' => (string)$now, 'id' => ++$this->id]);
		return $this->id;
	}

	protected function handle_pong($dat) {
		$now = (int)(microtime(true)*1000000);
		$diff = $now - $dat['ts'];
		printf("PONG received, lag is %01.3fms\n", $diff/1000);
	}

	public function loop() {
		while(!feof($this->fp)) {
			$this->handle();
		}
	}

	public function handle() {
		$pkt = $this->readPacket();
		if (!$pkt) return $pkt;

		$cmd = 'handle_'.$pkt['command'];
		if (is_callable([$this,$cmd])) {
			return $this->$cmd($pkt);
		} else {
			var_dump($pkt);
			return false;
		}
	}

	public function sendPacket($cmd) {
		$cmd = json_encode($cmd);
		fwrite($this->fp, pack('N', strlen($cmd)).$cmd);
	}

	public function readPacket() {
		$len = fread($this->fp, 4);
		list(,$len) = unpack('N', $len);
		if ($len == 0) return false;
		return json_decode(fread($this->fp, $len), true);
	}
}

