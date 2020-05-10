//
// Created by ronyshchuk on 07.04.20.
//

#include "text_view.hpp"
#include "cursor.hpp"
#include "font.hpp"
#include "page.hpp"
#include "window.hpp"

#include <algorithm>

TextView::TextView(const GlyphParams& params, Gui::Window* w):
    Gui::Window(params, w),
    m_visibleArea(0, 0, params.width, params.height)
{
    SetForeground(Color::kWhite);
    m_currentPage = std::make_shared<Page>(this, GlyphParams{0, 0, m_params.width - 1, pageHeight});
    ICompositeGlyph::Add(m_currentPage);
    m_visiblePages.push_back(m_currentPage);
    m_isVisible = false;
}

void TextView::Draw(Gui::Window* window)
{
    if(!m_isVisible) {
        this->ShowWindow();
    }
    for(auto& page: m_visiblePages) {
        page->Draw(this);
        // Draw page separator
        auto separatorParams = page->GetGlyphParams();
        separatorParams.y = separatorParams.y + separatorParams.height + 1;
        separatorParams.height = pageSeparator - 1;
        FillRectangle(separatorParams, Color::kGray);
    }

    m_currentPage->DrawCursor(this);
}

void TextView::ProcessEvent(Gui::Window* window, const Event& event)
{
    if(event.GetEvent() == EventType::KeyPressed) {
        m_currentPage->ProcessEvent(window, event);
        UpdateVisibleArea(window);

        return;
    } else if(event.GetEvent() == EventType::Scroll) {
        assert(m_onScroll);
        m_onScroll(event);
    }
}

void TextView::FillRectangle(const GlyphParams& params, const Color color)
{
    if(m_visibleArea.IsIntersects(params) || params.IsIntersects(m_visibleArea)) {
        m_window_impl->FillRectangle({params.x, params.y - m_visibleArea.y}, params.width, params.height, color);
    }
}

void TextView::DrawRectangle(const GlyphParams& params) const
{
    if(m_visibleArea.IsIntersects(params) || params.IsIntersects(m_visibleArea)) {
        m_window_impl->DrawRectangle({params.x, params.y - m_visibleArea.y}, params.width, params.height);
    }
}

void TextView::DrawText(const Point& text_position, std::string text) const
{
    if(m_visibleArea.IsIntersects({text_position.x, text_position.y})) {
        m_window_impl->DrawText({text_position.x, text_position.y - m_visibleArea.y}, text);
    }
}

void TextView::DrawLine(const Point& start_point, const Point& end_point) const
{
    // TODO(rmn):intersection
    m_window_impl->DrawLine(
        {start_point.x, start_point.y - m_visibleArea.y}, {end_point.x, end_point.y - m_visibleArea.y});
}

std::shared_ptr<Page> TextView::AddPage(const GlyphPtr& currentPage)
{
    assert(m_onPageAdded);

    if(currentPage == m_components.back()) {
        auto newPageParams = currentPage->GetGlyphParams();
        newPageParams.y = newPageParams.y + newPageParams.height + pageSeparator;

        auto newPage = std::make_shared<Page>(this, newPageParams);
        m_components.push_back(newPage);
        return newPage;
    } else {
        //        auto it = std::find(m_components.cbegin(), m_components.cend(), currentPage);
        //        if( it)
        //            m_components.insert(it)
    }
    return m_currentPage;
}

// TODO(rmn): optimize
void TextView::UpdateVisiblePages()
{
    m_visiblePages.clear();
    for(auto& it: m_components) {
        auto& pageParams = it->GetGlyphParams();
        if(m_visibleArea.IsIntersects(pageParams) || pageParams.IsIntersects(m_visibleArea)) {
            m_visiblePages.push_back(it);
        }
    }
}

void TextView::UpdateVisibleArea(Gui::Window* window)
{
    assert(m_onVisibleAreaUpdate);

    const auto chunkSize = Lexi::FontManager::Get().GetCharHeight() * 5;
    if(Lexi::Cursor::Get().GetCursorEnd() < m_visibleArea.y) {
        if(m_visibleArea.y < chunkSize) {
            m_visibleArea.y = 0;
        } else {
            m_visibleArea.y -= chunkSize;
        }

        UpdateVisiblePages();
        ReDraw(window);
        m_onVisibleAreaUpdate();
    }

    if(Lexi::Cursor::Get().GetCursorEnd() > m_visibleArea.y + m_visibleArea.height) {
        if(m_visibleArea.y + m_visibleArea.height + chunkSize > GetOverallHeight()) {
            m_visibleArea.y = GetOverallHeight() - m_visibleArea.height;
        } else {
            m_visibleArea.y += chunkSize;
        }

        UpdateVisiblePages();
        ReDraw(window);
        m_onVisibleAreaUpdate();
    }
}

void TextView::UpdateVisibleArea(height_t h)
{
    m_visibleArea.y = h;

    UpdateVisiblePages();  // TODO(rmn): fix
    ReDraw(this);
}

std::shared_ptr<Page> TextView::SwitchPage(Gui::Window* window, SwitchDirection direction, bool createIfNotExists)
{
    // TODO(need) update
    auto currentPageIt = std::find(m_components.begin(), m_components.end(), m_currentPage);
    if(direction == SwitchDirection::kNext) {
        if(m_currentPage == m_components.back()) {
            if(createIfNotExists) {
                m_currentPage = AddPage(m_currentPage);
                m_currentPage->Draw(window);
                UpdateVisibleArea(window);
                m_onPageAdded();
            } else {
                return m_currentPage;
            }
        } else {
            m_currentPage = std::static_pointer_cast<Page>(*(std::next(currentPageIt)));
        }
    } else {
        if(m_currentPage != m_components.front()) {
            m_currentPage = std::static_pointer_cast<Page>(*(std::prev(currentPageIt)));
        }
    }
    m_currentPage->DrawCursor(window);
    return m_currentPage;
}
