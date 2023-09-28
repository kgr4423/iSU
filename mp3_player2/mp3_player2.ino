#include <SDHCI.h>
#include <Audio.h>

SDClass theSD;
AudioClass *theAudio;

File audioFile;

bool ErrEnd = false;


void setup()
{
  SdSetup();

  err_t err = AudioSetup();

  /* Open file placed on SD card */
  audioFile = theSD.open("mp3/sample.mp3");

  /* Verify file open */
  if (!audioFile)
    {
      printf("File open error\n");
      exit(1);
    }
  printf("Open! 0x%08lx\n", (uint32_t)audioFile);

  /* Send first frames to be decoded */
  err = theAudio->writeFrames(AudioClass::Player0, audioFile);

  if ((err != AUDIOLIB_ECODE_OK) && (err != AUDIOLIB_ECODE_FILEEND))
    {
      printf("File Read Error! =%d\n",err);
      audioFile.close();
      exit(1);
    }

  puts("Play!");

  /* Main volume set to -16.0 dB */
  theAudio->setVolume(-600);
  theAudio->startPlayer(AudioClass::Player0);
}

void loop()
{
  puts("loop!!");

  /* Send new frames to decode in a loop until file ends */
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

  /* This sleep is adjusted by the time to read the audio stream file.
   * Please adjust in according with the processing contents
   * being processed at the same time by Application.
   *
   * The usleep() function suspends execution of the calling thread for usec
   * microseconds. But the timer resolution depends on the OS system tick time
   * which is 10 milliseconds (10,000 microseconds) by default. Therefore,
   * it will sleep for a longer time than the time requested here.
   */

  usleep(40000);


  /* Don't go further and continue play */
  return;

stop_player:
  theAudio->stopPlayer(AudioClass::Player0);
  audioFile.close();
  theAudio->setReadyMode();
  theAudio->end();
  exit(1);
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

err_t AudioSetup(){
  // start audio system
  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* Set clock mode to normal */
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);

  /* Set output device to speaker with first argument.
   * If you want to change the output device to I2S,
   * specify "AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT" as an argument.
   * Set speaker driver mode to LineOut with second argument.
   * If you want to change the speaker driver mode to other,
   * specify "AS_SP_DRV_MODE_1DRIVER" or "AS_SP_DRV_MODE_2DRIVER" or "AS_SP_DRV_MODE_4DRIVER"
   * as an argument.
   */
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);

  /*
   * Set main player to decode stereo mp3. Stream sample rate is set to "auto detect"
   * Search for MP3 decoder in "/mnt/sd0/BIN" directory
   */
  err_t err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);

  /* Verify player initialize */
  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("Player0 initialize error\n");
      exit(1);
    }

  return err;
}

/**
 * @brief Audio attention callback
 *
 * When audio internal error occurc, this function will be called back.
 */

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
    {
      ErrEnd = true;
   }
}