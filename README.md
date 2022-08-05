### CCamera
CCamera是基于FFmpeg 4.3开发的音视频通过RTMP推流的App。
使用流程：修改RecordParam类的server为推流地址，例如 rtmp://**/live/test，rtmp://**/hls/test，**为域名端口(live.tecent.com:8888)

实现：使用cameraX收集视频数据，AudioRecorder收集音频数据，分别放入音频队列、视频队列，构建编码线程处理音频帧、视频帧的编码、封装、推流。

