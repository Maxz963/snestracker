#include "BpmSpdAddWidget.h"
#include "Tracker.h"
#include "Pattern.h"

BpmSpdAddWidget::BpmSpdAddWidget(Tracker *tracker, PatternEditorPanel *pep) :
  bpm_cbuf("120"),
  bpm_title("BPM"),
  bpm_valtext(bpm_cbuf),
  bpm_incbtn("+", incbpm, this),
  bpm_decbtn("-", decbpm, this),

  spd_cbuf("06"),
  spd_title("Spd"),
  spd_valtext(spd_cbuf),
  spd_incbtn("+", incspd, this),
  spd_decbtn("-", decspd, this),

  add_cbuf("04"),
  add_title("Add"),
  add_valtext(add_cbuf),
  add_incbtn("+", incadd, this),
  add_decbtn("-", decadd, this),

  tracker(tracker),
  pep(pep)
{
  updatebpm();
  updatespd();
  updateadd();
}

/* these functions query the proper handles on that real data. */
void BpmSpdAddWidget :: updatebpm()
{
  sprintf(bpm_cbuf, "%03d", tracker->bpm);
}

void BpmSpdAddWidget :: updatespd()
{
  sprintf(spd_cbuf, "%02d", tracker->spd);
}

void BpmSpdAddWidget :: updateadd()
{
  sprintf(add_cbuf, "%02d", pep->addval);
}

void BpmSpdAddWidget :: set_coords(int x, int y)
{

  /* (x,y)
   * BPM  123 +-
   * Spd   03 +-
   * Add   02 +-*/
  int xx = x;
  bpm_title.rect.x = x;
  bpm_title.rect.y = y;
  bpm_valtext.rect.x = x + ((3 + 1) * CHAR_WIDTH);
  bpm_valtext.rect.y = y;
  bpm_incbtn.rect.x  = bpm_valtext.rect.x + (4 * CHAR_WIDTH);
  bpm_incbtn.rect.y = y;
  bpm_decbtn.rect.x = bpm_incbtn.rect.x + CHAR_WIDTH + 5;
  bpm_decbtn.rect.y = y;

  y += CHAR_HEIGHT + 5;

  spd_title.rect.x = x;
  spd_title.rect.y = y;
  spd_valtext.rect.x = x + ((3 + 2) * CHAR_WIDTH);
  spd_valtext.rect.y = y;
  spd_incbtn.rect.x  = spd_valtext.rect.x + (3 * CHAR_WIDTH);
  spd_incbtn.rect.y = y;
  spd_decbtn.rect.x = spd_incbtn.rect.x + CHAR_WIDTH + 5;
  spd_decbtn.rect.y = y;

  y += CHAR_HEIGHT + 5;

  add_title.rect.x = x;
  add_title.rect.y = y;
  add_valtext.rect.x = x + ((3 + 2) * CHAR_WIDTH);
  add_valtext.rect.y = y;
  add_incbtn.rect.x  = add_valtext.rect.x + (3 * CHAR_WIDTH);
  add_incbtn.rect.y = y;
  add_decbtn.rect.x = add_incbtn.rect.x + CHAR_WIDTH + 5;
  add_decbtn.rect.y = y;
}

int BpmSpdAddWidget::handle_event(const SDL_Event &ev)
{
  bpm_incbtn.check_event(ev);
  bpm_decbtn.check_event(ev);

  spd_incbtn.check_event(ev);
  spd_decbtn.check_event(ev);

  add_incbtn.check_event(ev);
  add_decbtn.check_event(ev);
}

//void BpmSpdAddWidget::one_time_draw(SDL_Surface *screen/*=::render->screen*/)
//{
//}

void BpmSpdAddWidget::draw(SDL_Surface *screen/*=::render->screen*/)
{
  bpm_title.draw(screen);
  bpm_valtext.draw(screen);
  bpm_incbtn.draw(screen);
  bpm_decbtn.draw(screen);
  spd_title.draw(screen);
  spd_valtext.draw(screen);
  spd_incbtn.draw(screen);
  spd_decbtn.draw(screen);

  updateadd();
  add_title.draw(screen);
  add_valtext.draw(screen);
  add_incbtn.draw(screen);
  add_decbtn.draw(screen);
}

int BpmSpdAddWidget::incbpm(void *bsaw)
{
  BpmSpdAddWidget *b = (BpmSpdAddWidget *)bsaw;
  b->tracker->inc_bpm();
  b->updatebpm();
}

int BpmSpdAddWidget::decbpm(void *bsaw)
{
  BpmSpdAddWidget *b = (BpmSpdAddWidget *)bsaw;
  b->tracker->dec_bpm();
  b->updatebpm();
}

int BpmSpdAddWidget::incspd(void *bsaw)
{
  BpmSpdAddWidget *b = (BpmSpdAddWidget *)bsaw;
  b->tracker->inc_spd();
  b->updatespd();
}

int BpmSpdAddWidget::decspd(void *bsaw)
{
  BpmSpdAddWidget *b = (BpmSpdAddWidget *)bsaw;
  b->tracker->dec_spd();
  b->updatespd();
}

int BpmSpdAddWidget::incadd(void *bsaw)
{
  BpmSpdAddWidget *b = (BpmSpdAddWidget *)bsaw;
  b->pep->inc_addval();
}

int BpmSpdAddWidget::decadd(void *bsaw)
{
  BpmSpdAddWidget *b = (BpmSpdAddWidget *)bsaw;
  b->pep->dec_addval();
}

