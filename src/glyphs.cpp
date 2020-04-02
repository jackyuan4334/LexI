//
// Created by romaonishuk on 28.10.19.
//

#include "i_glyph.hpp"

#include "types.hpp"
#include "window.hpp"

void IGlyph::ClearGlyph(Gui::Window *w)
{
    w->ClearArea(m_params);
}

// --------------------------------------------

///// ---- Character ---
// void Character::Draw(Gui::Window *w)
//{
//    assert(w);
//    w->DrawRectangle({33, 22}, 123, 124);
//}
//
//
///// --- Row ---
// void Row::Draw(Gui::Window *w)
//{
//    assert(w);
//    w->DrawRectangle({33, 22}, 400, 500);
//}
