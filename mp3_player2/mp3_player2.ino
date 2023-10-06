#include <SDHCI.h>
#include <Audio.h>

SDClass theSD;
AudioClass *theAudio; //ポインタ

File audioFile;

bool ErrEnd = false;

void setup()
{
  SdSetup();

  AudioSetup();
  openAudioFileInSd("mp3/sample.mp3");
  sendFirstFramesOfAudioFile();
  theAudio->setVolume(-600); //-60dB
  theAudio->startPlayer(AudioClass::Player0);
}

void loop()
{
  sendNextFramesOfAudioFile();
  exit(0);
}

//
// SDの初期化
//
void SdSetup(){
  while (!theSD.begin()) 
    {
      //SDカードが挿入されるまで待つ
      Serial.println("Insert SD card!");
    }
  Serial.println("SD Prepared");
}

void AudioSetup(){
  // start audio system
  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* クロックモードをノーマルに設定する */
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);

  //第一引数で出力デバイスをスピーカーに設定している
  //第2引数でスピーカードライバモードを LineOut に設定している
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);

  /*
   * メインプレーヤーがステレオMP3をデコードするように設定する。ストリームのサンプルレートは "自動検出 "に設定されています。
   * mnt/sd0/BIN "ディレクトリでMP3デコーダーを検索する。
   */
  err_t err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("Player0 initialize error\n");
      exit(1);
    }
}

void openAudioFileInSd(char* mp3Filepath)
{
  /* Open file placed on SD card */
  audioFile = theSD.open(mp3Filepath);

  /* Verify file open */
  if (!audioFile)
    {
      printf("File open error\n");
      exit(1);
    }
  printf("Open! 0x%08lx\n", (uint32_t)audioFile);
}

void sendFirstFramesOfAudioFile()
{
  /* Send first frames to be decoded */
  err_t err = theAudio->writeFrames(AudioClass::Player0, audioFile);

  if ((err != AUDIOLIB_ECODE_OK) && (err != AUDIOLIB_ECODE_FILEEND))
    {
      printf("File Read Error! =%d\n",err);
      audioFile.close();
      exit(1);
    }
}

void sendNextFramesOfAudioFile(){
loop:
  /* ファイル終了までループで新しいフレームをデコードに送る */
  int err = theAudio->writeFrames(AudioClass::Player0, audioFile);

  /*  Tell when player file ends */
  if (err == AUDIOLIB_ECODE_FILEEND)
    {
      printf("Main player File End!\n");
    }

  /* Show error code from player and stop */
  if (err)
    {
      printf("Main player error code: %d\n", err);
      goto stop_player;
    }

  if (ErrEnd)
    {
      printf("Error End\n");
      goto stop_player;
    }

  //このスリープは、オーディオストリームファイルの読み込み時間によって調整されます。
  //アプリケーションで同時に処理される処理内容に応じて調整してください。
  usleep(40000);

  goto loop;

stop_player:
  theAudio->stopPlayer(AudioClass::Player0);
  audioFile.close();
  theAudio->setReadyMode();
  theAudio->end();
  // exit(1); //loopの終了
  return;
}

/**
 * @brief Audio attention callback
 *
 * オーディオ内部エラーが発生すると、この関数がコールバックされる。
 */
static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}