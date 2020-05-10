//
// Created by ronyshchuk on 19.04.20.
//

#include "page.hpp"

#include <algorithm>

#include "character.hpp"
#include "cursor.hpp"
#include "font.hpp"
#include "row.hpp"
#include "text_view.hpp"

height_t Page::m_topIndent = 50;
height_t Page::m_botIndent = 25;
width_t Page::m_leftIndent = 50;
width_t Page::m_rightIndent = 75;

Page::Page(TextView* parent, const GlyphParams& params): ICompositeGlyph(params), m_parent(parent)
{
    m_currentRow = std::make_shared<Row>(
        GlyphParams{m_leftIndent, m_params.y + m_topIndent + Lexi::FontManager::Get().GetCharHeight(),
            m_params.width - m_leftIndent - m_rightIndent, Lexi::FontManager::Get().GetCharHeight()});
    ICompositeGlyph::Add(m_currentRow);
}

void Page::Draw(Gui::Window* window)
{
    window->FillRectangle(m_params, Color::kWhite);
    window->SetForeground(Color::kBlack);
    window->DrawRectangle(m_params);

    ICompositeGlyph::Draw(window);
}

void Page::DrawCursor(Gui::Window* window)
{
    m_currentRow->DrawCursor(window);
}

// void Page::UpdateContent()
//{
//
//}

void Page::ProcessEvent(Gui::Window* window, const Event& event)
{
    // TODO(rmn): oh my, need to add event/notification mechanism
    const auto* key = static_cast<const Lexi::KeyBoardEvent*>(&event);
    auto& cursor = Lexi::Cursor::Get();

    if(key->IsString()) {
        auto res = m_currentRow->AddCharacter(window, key->m_key);
        if(res) {
            auto c = std::static_pointer_cast<Character>(*res)->GetChar();
            auto newEvent = Lexi::KeyBoardEvent(event.GetPoint(), c);
            if(m_currentRow == m_components.back()) {
                if(IsLastRow(m_currentRow)) {
                    auto nextPage = m_parent->SwitchPage(window, TextView::SwitchDirection::kNext, true);
                    nextPage->ProcessEvent(window, newEvent);
                } else {
                    m_currentRow = std::make_shared<Row>(GlyphParams{m_leftIndent,
                        m_currentRow->GetPosition().y + Lexi::FontManager::Get().GetCharHeight(),
                        m_params.width - m_leftIndent - m_rightIndent, Lexi::FontManager::Get().GetCharHeight()});
                    ICompositeGlyph::Add(m_currentRow);
                    ProcessEvent(window, newEvent);
                }
            } else {
                m_currentRow = GetNextRow();
                cursor.MoveCursor(window, m_currentRow->GetGlyphParams());
                ProcessEvent(window, newEvent);
            }
        }
        return;
    }

    switch(static_cast<Lexi::Key>(key->m_key)) {
        case Lexi::Key::kEnter: {
            auto fontHeight = Lexi::FontManager::Get().GetCharHeight();
            if(m_currentRow->GetPosition().y + fontHeight < m_params.y + m_params.height - m_botIndent) {
                m_currentRow = std::make_shared<Row>(
                    GlyphParams{m_params.x + m_leftIndent, m_currentRow->GetPosition().y + fontHeight,
                        m_params.width - m_leftIndent - m_rightIndent, fontHeight});
                Add(m_currentRow);
                m_currentRow->DrawCursor(window);
            } else {
                auto nextPage = m_parent->SwitchPage(window, TextView::SwitchDirection::kNext, true);
                //                nextPage->Ad
            }
        } break;
        case Lexi::Key::kArrowUp: {
            if(m_currentRow == m_components.front()) {
                m_parent->SwitchPage(window, TextView::SwitchDirection::kPrev, true);
            } else {
                m_currentRow = GetPreviousRow();
                m_currentRow->DrawCursor(window);
            }
        } break;
        case Lexi::Key::kArrowDown: {
            if(m_currentRow == m_components.back()) {
                m_parent->SwitchPage(window, TextView::SwitchDirection::kNext, false);
            } else {
                m_currentRow = GetNextRow();
                m_currentRow->DrawCursor(window);
            }
        } break;
        case Lexi::Key::kArrowLeft: {
            if(Lexi::Cursor::Get().GetPosition().x == m_currentRow->GetPosition().x) {
                // switch page
                if(m_currentRow == m_components.front()) {
                } else {  // move cursor to previous row
                    m_currentRow = GetPreviousRow();
                    m_currentRow->DrawCursorAtEnd(window);
                }
            } else {
                m_currentRow->ProcessEvent(window, event);
            }
        } break;
        case Lexi::Key::kArrowRight: {
            auto lastChar = m_currentRow->GetLastChar();
            if((lastChar && lastChar->GetRightBorder() == Lexi::Cursor::Get().GetPosition().x) ||
                m_currentRow->IsEmpty()) {
                if(m_currentRow == m_components.back()) {
                    // switch page
                } else {
                    m_currentRow = GetNextRow();
                    Lexi::Cursor::Get().MoveCursor(window, m_currentRow->GetGlyphParams());
                }
            } else {
                m_currentRow->ProcessEvent(window, event);
            }
        } break;
        case Lexi::Key::kBackspace:
            if(Lexi::Cursor::Get().GetPosition().x == m_currentRow->GetPosition().x) {
                ProcessBackspace(window);
            } else {
                m_currentRow->ProcessEvent(window, event);
            }
            break;
        default:
            break;
    }
}

bool Page::IsLastRow(const GlyphPtr& row) const
{
    return m_currentRow->GetBottomBorder() + Lexi::FontManager::Get().GetCharHeight() >
        m_params.y + m_params.height - m_botIndent;
}

std::shared_ptr<Row> Page::GetPreviousRow()
{
    auto rowIt = std::find(m_components.begin(), m_components.end(), m_currentRow);
    return std::static_pointer_cast<Row>(*(std::prev(rowIt)));
}

std::shared_ptr<Row> Page::GetNextRow()
{
    auto rowIt = std::find(m_components.begin(), m_components.end(), m_currentRow);
    return std::static_pointer_cast<Row>(*std::next(rowIt));
}

void Page::ProcessBackspace(Gui::Window* window)
{
    if(m_currentRow == m_components.front()) {
        auto previousPage = m_parent->SwitchPage(window, TextView::SwitchDirection::kPrev, false);
        if(!previousPage) {
            m_currentRow->DrawCursor(window);
            return;
        }
        return;
    }

    auto prevRowIt = std::prev(std::find(m_components.begin(), m_components.end(), m_currentRow));
    auto previousRow = std::static_pointer_cast<Row>(*prevRowIt);

    if(m_currentRow->IsEmpty()) {
        m_components.remove(m_currentRow);
        m_currentRow = previousRow;
        m_currentRow->DrawCursor(window);

        // Update the rows below
        auto it = std::find(m_components.begin(), m_components.end(), previousRow);
        for(++it; it != m_components.end(); ++it) {
            auto& row = *it;
            const auto& position = row->GetPosition();
            row->SetPosition(position.x, position.y - m_currentRow->GetHeight());
        }

        // TODO(rmn): Update all other rows in other pages
        return;
    }

    // Replace last element of previous row with the first element of current
    if(previousRow->IsFull()) {
        auto removedElement = previousRow->GetLastChar();

        removedElement->SetChar(m_currentRow->GetFirstChar()->GetChar());
        removedElement->ReDraw(window);

        m_currentRow->Remove(window, m_currentRow->GetFirstChar());
        m_currentRow = previousRow;
        m_currentRow->DrawCursor(window);
    } else {
        auto prevRowCapacity = previousRow->GetFreeSpace();
        if(prevRowCapacity >= m_currentRow->GetUsedSpace()) {
            previousRow->Insert(m_currentRow);
            if(m_currentRow == m_components.back()) {
                if(IsLastRow(m_currentRow)) {
                    // TODO(rmn): recursively remove lines from all pages
                } else {
                    m_currentRow->ClearGlyph(window);
                    window->FillRectangle(m_currentRow->GetGlyphParams(), Color::kWhite);

                    m_components.remove(m_currentRow);
                    m_currentRow = previousRow;
                    m_currentRow->DrawCursorAtEnd(window);
                }
            }
        } else {
            auto lst = m_currentRow->Cut(m_currentRow->GetPosition().x, prevRowCapacity);
            previousRow->Insert(previousRow->GetLastChar()->GetRightBorder(), std::move(lst));
            m_currentRow = previousRow;
            m_currentRow->DrawCursor(window);
        }
    }
}
