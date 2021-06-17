#include "Menu_Bar.h"
#include "utility.h"
#include "platform.h"
#include "Options_Window.h"
#include "gui/Spc_Export_Window.h"

#include "gme/Wave_Writer.h"
#include "Tracker.h"
#include "apuram.h"

#include "shared/SdlNfd.h"
#include "shared/RecentFiles.h"

extern Tracker *tracker;

Menu_Bar::Menu_Bar()
{
}

Menu_Bar::~Menu_Bar()
{
}

int Menu_Bar::Edit_Context::copy(void *data)
{
  // STUB
}

int Menu_Bar::Edit_Context::paste(void *data)
{
  // STUB
}

int Menu_Bar::Edit_Context::open_options_window(void *data)
{
  ::options_window->show();
  ::options_window->raise();
  return 0;
}

void Menu_Bar::draw(SDL_Surface *screen)
{
  if (is_first_run)
  {
    context_menus.preload(10, 10);
    is_first_run = false;
  }

  context_menus.draw(screen);
}

static void updateRecentFiles(Menu_Bar::File_Context *fc)
{
  /* Check for populated strings */
  for (int i=0; i < NUM_RECENTFILES; i++)
  {
    if (RecentFiles::paths[i] != NULL)
    {
      fc->menu_items[RECENTFILE_STARTIDX - 2].is_visible = true; // enable the recentfiles HEAD spacer
      fc->menu_items[RECENTFILE_STARTIDX - 1].is_visible = true; // enable the recentfiles HEADLINE
      fc->menu_items[RECENTFILE_STARTIDX + NUM_RECENTFILES].is_visible = true; // enable the end-of-recentfiles separator
      fc->menu_items[RECENTFILE_STARTIDX + i].is_visible = true; // enable the entry
      fc->menu_items[RECENTFILE_STARTIDX + i].clickable_text.str = RecentFiles::dnames[i];
      fc->menu_items[RECENTFILE_STARTIDX + i].clickable_text.init_width_height();
    }
    else
    {
      fc->menu_items[RECENTFILE_STARTIDX + i].is_visible = false; // enable the entry
    }
  }

  fc->menu.preload(fc->menu.created_at.x, fc->menu.created_at.y);
}

int Menu_Bar::File_Context::openRecent(void *i)
{
  intptr_t idx = (intptr_t) i;
  const char *path = RecentFiles::paths[idx];

  if (::tracker->song.changed)
  {
    int rc = ::tracker->DialogUnsavedChanges();
    //DEBUGLOG("rc = %d\n", rc);
    if (rc == -1) // file didn't save, or user pressed cancel
      return -1;
    // 0 means file saved or user pressed no saving
  }

  SDL_RWops *file = SDL_RWFromFile(path, "r");
  if (file == NULL)
  {
    char tmpbuf[500];
    sprintf(tmpbuf, "Warning: Unable to open file %s!\n %s", path,
            SDL_GetError() );
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                   "Could not open FILE!",
                   tmpbuf,
                   NULL);
    // TODO
    RecentFiles::remove(idx);
    updateRecentFiles(&::tracker->menu_bar.context_menus.file_context);

    return NFD_ERROR;
  }

  auto rc = ::tracker->read_from_file(file);

  SDL_RWclose(file);

  if (rc == 0)
  {
    /*Successful. Update the current song's filepath */
    strncpy(::tracker->menu_bar.context_menus.file_context.filepath, path, 500);
    /* put the filename in the window title */
    ::tracker->updateWindowTitle( Utility::getFileName(RecentFiles::paths[idx]) );    /* Add the filename to the RecentFiles list */
    RecentFiles::push(path); //psuedo-code
    /* Update the Open Recent Context Menu! */
    updateRecentFiles(&::tracker->menu_bar.context_menus.file_context);
  }

  return rc;
}

Menu_Bar::File_Context::File_Context() : menu(menu_items, true)
{
  memset(filepath, 0, 500);

  updateRecentFiles(this);
}

int Menu_Bar::File_Context::new_song(void *data)
{
	File_Context *fc = (File_Context *)data;

  if (::tracker->song.changed)
  {
    int rc = ::tracker->DialogUnsavedChanges();
    //DEBUGLOG("rc = %d\n", rc);
    if (rc == -1) // file didn't save, or user pressed cancel
      return -1;
    // 0 means file saved or user pressed no saving
  }

	fc->filepath[0] = 0;

	::tracker->reset();
	::tracker->song.patseq.num_entries = 1;
	::tracker->song.patseq.sequence[0] = 0;
	::tracker->song.patseq.patterns[0].used = 1;
}

int Menu_Bar::File_Context::open_song(void *data)
{
	File_Context *fc = (File_Context *)data;

  if (::tracker->song.changed)
  {
    int rc = ::tracker->DialogUnsavedChanges();
    //DEBUGLOG("rc = %d\n", rc);
    if (rc == -1) // file didn't save, or user pressed cancel
      return -1;
    // 0 means file saved or user pressed no saving
  }

  //DEBUGLOG("NOw open\n");
	/* Open the file */
	nfdresult_t rc = SdlNfd::get_file_handle("r", SONGFILE_EXT);
	if (rc != NFD_OKAY)
		return rc;

	rc = (nfdresult_t) ::tracker->read_from_file(SdlNfd::file);

	if (rc == 0)
	{
		/*Successful. Update the current song's filepath */
		strncpy(fc->filepath, SdlNfd::outPath, 500);
    /* put the filename in the window title */
    ::tracker->updateWindowTitle( Utility::getFileName(fc->filepath) );
    /* Add the filename to the RecentFiles list */
    RecentFiles::push(fc->filepath); //psuedo-code
    /* Update the Open Recent Context Menu! */
    updateRecentFiles(fc);
	}

	return rc;
}

int Menu_Bar::File_Context::open_spc(void *data)
{
  ::player->pause(1,true,false);

  if (::nfd.get_multifile_read_path("spc,rsn,rar,7z") == NFD_OKAY)
  {
  }

  ::player->pause(0);
  return 0;
}

static int save_common(Menu_Bar::File_Context *fc, SDL_RWops *file, nfdchar_t *filepath)
{
	::tracker->save_to_file(file);

	strncpy(fc->filepath, filepath, 500);

  /* Add the filename to the RecentFiles list */
  RecentFiles::push(filepath); //psuedo-code
  /* Update the Open Recent Context Menu! */
  updateRecentFiles(&::tracker->menu_bar.context_menus.file_context);

  /* put the filename in the window title */
  ::tracker->updateWindowTitle( Utility::getFileName( filepath ) );
	return NFD_OKAY;
}

int Menu_Bar::File_Context::save_song(void *data)
{
	File_Context *fc = (File_Context *)&tracker->menu_bar.context_menus.file_context;
  int rc = 0;

	if (fc->filepath[0] == 0)
	{
		if (SdlNfd::get_file_handle("w", SONGFILE_EXT) != NFD_OKAY)
			return -1;

    DEBUGLOG("attempting to save to new file: %s\n", SdlNfd::outPath);
    rc = save_common(fc, SdlNfd::file, SdlNfd::outPath);
    if (rc != NFD_OKAY)
      return -1;
	}
	else
  {
    DEBUGLOG("attempting to save to current file: %s\n", fc->filepath);
    SDL_RWops *file = SDL_RWFromFile(fc->filepath, "w");
    if (file == NULL)
    {
      char tmpbuf[250];
      sprintf(tmpbuf, "Warning: Unable to open file %s!\n %s", fc->filepath, SDL_GetError() );
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
          "Could not open FILE!",
         tmpbuf,
          NULL);
      return -1;
    }

    rc = save_common(fc, file, fc->filepath);

    // Manually close the file handle since it was manually created in one case (save_song else path)
    SDL_RWclose(file);
  }

  return rc;
}

int Menu_Bar::File_Context::save_as_song(void *data)
{
	File_Context *fc = (File_Context *)&tracker->menu_bar.context_menus.file_context;

	if (SdlNfd::get_file_handle("w", SONGFILE_EXT) != NFD_OKAY)
    return -1;

  DEBUGLOG("attempting to save to new file: %s\n", SdlNfd::outPath);
	return save_common(fc, SdlNfd::file, SdlNfd::outPath);
}

int Menu_Bar::File_Context::export_spc(void *data)
{
  // Do not show the window if the player has nothing loaded
  ::tracker->playback = false;
  ::player->fade_out(false); // immediate fade-out (no thread)
	::tracker->render_to_apu();
	::spc_export_window->show();

  return 0;
}


// This is the same func as Debugger's Menu_Bar. should rename the
// Debugger Menu class so we can reference those funcs
int Menu_Bar::File_Context::export_wav(void *data)
{
  nfdchar_t *outPath=NULL;

  nfdresult_t result = NFD_SaveDialog( "wav", NULL, &outPath );
  SDL_RaiseWindow(::render->sdlWindow);

  if ( result == NFD_OKAY )
  {
    if (outPath !=NULL)
      fprintf(stderr, "%s\n", outPath);

    /* Begin writing to wave file */
    wave_open( ::player->sample_rate, outPath );
    wave_enable_stereo();

    ::player->exporting = true;
    //BaseD::reload();
    ::player->pause(0, false, false);

/* FIXME TODO
    while ( (::player->emu()->tell()/1000) < BaseD::song_time )
    {
      const unsigned int buf_size = 1024 // can be any multiple of 2
      sample_t buf [buf_size];

      Music_Player::fill_buffer(::player, buf, buf_size);

      wave_write( buf, buf_size );
    }
*/
    free(outPath);
    wave_close();
    ::player->exporting = false;
    return result;
  }
  else if ( result == NFD_CANCEL )
  {
    if (outPath)
      free(outPath);
    puts("User pressed cancel.");
    return result;
  }
  else
  {
    if (outPath)
      free(outPath);
    printf("Error: %s\n", NFD_GetError() );
    return NFD_ERROR;
  }
}

#define ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])

int Menu_Bar::File_Context::export_rom(void *data)
{
  // Populate APU RAM
  ::tracker->playback = false;
  ::player->fade_out(false); // immediate fade-out (no thread)
  ::tracker->render_to_apu();
  //::spc_export_window->show();

  // Get Directory tthingy
  /*nfdresult_t result = SdlNfd::get_directory();
  if ( result == NFD_OKAY )
  {
    DEBUGLOG("Directory Set: %s\n", SdlNfd::dirPath);
  }
  else
  {
    return -1;
  }*/

  /*
    Write a bin file that is a sequence of:
    Dest SPC Address, Data Size, Data
    ...
    ...
    .dw $0000, SPC_CODE_START

    Figure out the Destination SPC Addresses
  */

  // Write to payload to a file
  nfdresult_t result = SdlNfd::get_file_handle("w", "bin");

  if ( result == NFD_OKAY )
  {
    SDL_RWops *file = SdlNfd::file;
    uint16_t word;
    TrackerApuRam *apuram = ::tracker->apuram;

    fprintf(stderr, "Exporting ROM Payload to: %s\n", SdlNfd::outPath);

/* // CONCERN: LOROM $8000 per bank */

/// 1
    // First write the Non-internal PUBLIC SPC Driver variables.
    /* eg. Instrument Table location, Pattern table location, etc etc */
    // Make a note of the apuram->padding amount, so we know where to start
    // uploading to APU RAM

    // Dest SPC Address
    {
      apuram->extflags = 0;
      auto startAddr = ARRAY_SIZE(apuram->padding);
      word = startAddr;
      SDL_RWwrite(file, &word, 2, 1);
      // Data Size
      word = 0x10000 - ( ARRAY_SIZE(apuram->padding) + ARRAY_SIZE(apuram->padding2) );
      //DEBUGLOG("Bytes of Public APU RAM vars: 0x%04x\n", word);
      //DEBUGLOG("1: %d, 2: %d\n", ARRAY_SIZE(apuram->padding), ARRAY_SIZE(apuram->padding2));
      SDL_RWwrite(file, &word, 2, 1);
      // Data
      const char *raw = (const char *)apuram;
      SDL_RWwrite(file, &raw[ARRAY_SIZE(apuram->padding)], word, 1);
      DEBUGLOG("APU PUB RAM: 0x%04x - 0x%04x (0x%04x bytes)\n", startAddr, startAddr + word - 1, word);
    }


    /*
      TODO: dspdir_i WHAT IS THIS AGAIN? LOL
      patterntable_ptr
      sequencer_ptr
      instrtable_ptr
    */

/*
      ; DATA FORMAT
      ; (DW) Start Address to upload to SPC RAM, (DW) DATA SIZE
      ; Data to upload
*/
/// 1a) Upload the Driver
    word = SPCDRIVER_CODESTART;
    SDL_RWwrite(file, &word, 2, 1);
    word = SPCDRIVER_CODESIZE;
    SDL_RWwrite(file, &word, 2, 1);
    // Write the driver directly from IAPURAM
    SDL_RWwrite(file, &::IAPURAM[SPCDRIVER_CODESTART], SPCDRIVER_CODESIZE, 1);
    DEBUGLOG("Driver Block: 0x%04x - 0x%04x (0x%04x bytes)\n",
              SPCDRIVER_CODESTART, SPCDRIVER_CODESTART + SPCDRIVER_CODESIZE - 1, SPCDRIVER_CODESIZE);

/// 1b) Upload the Patterns Block (PatternLUT, patterns, Sequencer Table)
    auto PBSA = ::tracker->apuRender.patternsBlockStartAddress;
    word = PBSA;
    SDL_RWwrite(file, &word, 2, 1);
    auto PBS = ::tracker->apuRender.patternsBlockSize;
    word = PBS;
    SDL_RWwrite(file, &word, 2, 1);
    SDL_RWwrite(file, &::IAPURAM[PBSA], PBS, 1);
    DEBUGLOG("Pattern Block: 0x%04x - 0x%04x (0x%04x bytes)\n", PBSA, PBSA + PBS - 1, PBS);

/// 2 Samples & Instruments
    auto SDSA = ::tracker->apuRender.sampleDirStartAddress;
    word = SDSA;
    SDL_RWwrite(file, &word, 2, 1);
    auto SDS = ::tracker->apuRender.sampleDirSize;
    word = SDS;
    SDL_RWwrite(file, &word, 2, 1);
    SDL_RWwrite(file, &::IAPURAM[SDSA], SDS, 1);
    DEBUGLOG("SampleDir: 0x%04x - 0x%04x (0x%04x bytes)\n", SDSA, SDSA + SDS - 1, SDS);

    auto ITSA = ::tracker->apuRender.instrumentTableStartAddress;
    word = ITSA;
    SDL_RWwrite(file, &word, 2, 1);
    auto ITS = ::tracker->apuRender.instrumentTableSize;
    word = ITS;
    SDL_RWwrite(file, &word, 2, 1);
    SDL_RWwrite(file, &::IAPURAM[ITSA], ITS, 1);
    DEBUGLOG("Instrument Table: 0x%04x - 0x%04x (0x%04x bytes)\n", ITSA, ITSA + ITS - 1, ITS);

    auto SSA = ::tracker->apuRender.samplesStartAddress;
    word = SSA;
    SDL_RWwrite(file, &word, 2, 1);
    auto SS = ::tracker->apuRender.samplesSize;
    word = SS;
    SDL_RWwrite(file, &word, 2, 1);
    SDL_RWwrite(file, &::IAPURAM[SSA], SS, 1);
    DEBUGLOG("Samples: 0x%04x - 0x%04x (0x%04x bytes)\n", SSA, SSA + SS - 1, SS);

    auto ISA = ::tracker->apuRender.instrumentsStartAddress;
    word = ISA;
    SDL_RWwrite(file, &word, 2, 1);
    auto IS = ::tracker->apuRender.instrumentsSize;
    word = IS;
    SDL_RWwrite(file, &word, 2, 1);
    SDL_RWwrite(file, &::IAPURAM[ISA], IS, 1);
    DEBUGLOG("Instruments: 0x%04x - 0x%04x (0x%04x bytes)\n", ISA, ISA + IS - 1, IS);

    // Write the final instruction to jump SPC to driver start address
    word = 0;
    SDL_RWwrite(file, &word, 2, 1);
    word = SPCDRIVER_CODESTART;
    SDL_RWwrite(file, &word, 2, 1);

    //::player->pause(1, false, false);

    // Begin writing to ROM payload file
    // copy header info
    //unsigned char *out = sew->state;
    ///Spc_Emu::header_t *header = (Spc_Emu::header_t *)out;



    //::player->spc_emu()->can_has_apu()->save_spc(out);
    //::file_system->write_file( SdlNfd::outPath, sew->state, Snes_Spc::spc_file_size );
    // restore player state
    //::player->pause(1, false, false);
    //sew->hide();
  }
  // DONEZIES */
  return 0;
}


void Menu_Bar::Context_Menus::preload(int x/*=x*/, int y/*=y*/)
{
  this->x = x; this->y = y;

  file_context.menu.linespace = 4;
  edit_context.menu.linespace = 4;
  window_context.menu.linespace = 4;
  about_context.menu.linespace = 4;

  file_context.menu.hpadding = CHAR_WIDTH;
  edit_context.menu.hpadding = CHAR_WIDTH;
  window_context.menu.hpadding = CHAR_WIDTH;
  about_context.menu.hpadding = CHAR_WIDTH;

  file_context.menu.preload(x, y);
  x +=  ( strlen(file_context.menu_items[0].clickable_text.str)
          * CHAR_WIDTH ) + CHAR_WIDTH*2;

  edit_context.menu.preload(x, y);
  x +=  ( strlen(file_context.menu_items[0].clickable_text.str)
          * CHAR_WIDTH ) + CHAR_WIDTH*2;

  window_context.menu.preload(x,y);
  x +=  ( strlen(window_context.menu_items[0].clickable_text.str)
          * CHAR_WIDTH ) + CHAR_WIDTH*2;

  about_context.menu.preload(x,y);
  x +=  ( strlen(about_context.menu_items[0].clickable_text.str)
          * CHAR_WIDTH ) + CHAR_WIDTH*2;
}

bool Menu_Bar::Context_Menus::check_left_click_activate(int &x, int &y,
        const Uint8 &button, const SDL_Event *ev)
{
  if (file_context.menu.check_left_click_activate(x, y, button, ev))
  {
    edit_context.menu.deactivate();
    window_context.menu.deactivate();
    about_context.menu.deactivate();
    return true;
  }

  if (edit_context.menu.check_left_click_activate(x, y, button, ev))
  {
    file_context.menu.deactivate();
    window_context.menu.deactivate();
    about_context.menu.deactivate();
    return true;
  }

  if (window_context.menu.check_left_click_activate(x, y, button, ev))
  {
    file_context.menu.deactivate();
    edit_context.menu.deactivate();
    about_context.menu.deactivate();
    return true;
  }

  if (about_context.menu.check_left_click_activate(x, y, button, ev))
  {
    file_context.menu.deactivate();
    edit_context.menu.deactivate();
    window_context.menu.deactivate();
    return true;
  }

  return false;
}

#include "kbd.h"
int Menu_Bar::receive_event(SDL_Event &ev)
{ 
  int r;

  if (ev.type == SDL_MOUSEBUTTONDOWN)
  {
    if (!::player->has_no_song)
    {
      bool r = false; //tabs.check_mouse_and_execute(ev.button.x, ev.button.y);
      if (r) return r;
    }
  }
  else if (ev.type == SDL_KEYDOWN)
  {
    int scancode = ev.key.keysym.sym;
    int mod = ev.key.keysym.mod;
    switch (scancode)
    {
      case SDLK_o:
        if (MODONLY(mod, CMD_CTRL_KEY))
          File_Context::open_song(&context_menus.file_context);
      break;
      case SDLK_e:
        if (MODONLY(mod, CMD_CTRL_KEY))
          File_Context::export_spc(NULL);
      break;
    }
  }

  r = context_menus.receive_event(ev);
  if (r) return r;

  return EVENT_INACTIVE;
}

bool Menu_Bar::Context_Menus::is_anything_active()
{
  return (file_context.menu.is_active || edit_context.menu.is_active ||
    window_context.menu.is_active || about_context.menu.is_active);
}
int Menu_Bar::Context_Menus::receive_event(SDL_Event &ev)
{
  int r;
  if ( ev.type == SDL_MOUSEBUTTONDOWN  && ev.button.button == SDL_BUTTON_LEFT || is_anything_active() )
  {
    if (check_left_click_activate(ev.button.x, ev.button.y, ev.button.button, &ev))
    {
      return EVENT_ACTIVE;
    }
  }

  if ((r=file_context.menu.receive_event(ev)))
  {
    if (r == Expanding_List::EVENT_MENU)
      return EVENT_FILE;
    return EVENT_ACTIVE;
  }
  if ((r=edit_context.menu.receive_event(ev)))
  {
    if (r == Expanding_List::EVENT_MENU)
      return EVENT_EDIT;
    return EVENT_ACTIVE;
  }
  if (!::player->has_no_song)
  {
    if ((r=window_context.menu.receive_event(ev)))
    {
      if (r == Expanding_List::EVENT_MENU)
        return EVENT_WINDOW;
      return EVENT_ACTIVE;
    }
  }
  if ((r=about_context.menu.receive_event(ev)))
  {
    if (r == Expanding_List::EVENT_MENU)
      return EVENT_ABOUT;
    return EVENT_ACTIVE;
  }

  return EVENT_INACTIVE;
}
void Menu_Bar::Context_Menus::update(Uint8 adsr1, Uint8 adsr2)
{
  // Don't need this because there is no currently selected item, just actions
  /*file_context.menu.update_current_item(Menu_Bar::get_attack_index(adsr1));
    window_context.menu.update_current_item(Menu_Bar::get_sustain_level_index(adsr2));*/
}

void Menu_Bar::Context_Menus::draw(SDL_Surface *screen)
{
  file_context.menu.draw(screen);
  edit_context.menu.draw(screen);
  window_context.menu.draw(screen);
  about_context.menu.draw(screen);
}

void Menu_Bar::Context_Menus::deactivate_all()
{
  file_context.menu.deactivate();
  edit_context.menu.deactivate();
  window_context.menu.deactivate();
  about_context.menu.deactivate();
}

int Menu_Bar::About_Context::clicked_patreon(void *nada)
{
  DEBUGLOG("CLICKED PATREON\n");
  openUrl(PATREON_URL);
}

int Menu_Bar::About_Context::clicked_merch(void *nada)
{
  DEBUGLOG("CLICKED MERCH\n");
  openUrl(MERCH_URL);
}

int Menu_Bar::About_Context::clicked_soundcloud(void *nada)
{
  DEBUGLOG("CLICKED SOUNDCLOUD\n");
  openUrl(SOUNDCLOUD_URL);
}

int Menu_Bar::About_Context::clicked_wiki(void *nada)
{
  DEBUGLOG("CLICKED WIKI\n");
  openUrl(WIKI_URL);
}

int Menu_Bar::About_Context::clicked_stream(void *nada)
{
  DEBUGLOG("CLICKED STREAM\n");
  openUrl(STREAM_URL);
}
int Menu_Bar::About_Context::clicked_twitter(void *nada)
{
  DEBUGLOG("CLICKED TWITTER\n");
  openUrl(TWITTER_URL);
}
