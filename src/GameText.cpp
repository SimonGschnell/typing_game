#include "../include/GameText.hpp"
#include <iostream>

namespace{

    void addGlyphQuad(sf::VertexArray& vertices, sf::Vector2f position, const sf::Color& color, const sf::Glyph& glyph, float italicShear)
    {
        float padding = 1.0;

        float left   = glyph.bounds.left - padding;
        float top    = glyph.bounds.top - padding;
        float right  = glyph.bounds.left + glyph.bounds.width + padding;
        float bottom = glyph.bounds.top  + glyph.bounds.height + padding;

        float u1 = static_cast<float>(glyph.textureRect.left) - padding;
        float v1 = static_cast<float>(glyph.textureRect.top) - padding;
        float u2 = static_cast<float>(glyph.textureRect.left + glyph.textureRect.width) + padding;
        float v2 = static_cast<float>(glyph.textureRect.top  + glyph.textureRect.height) + padding;

        vertices.append(sf::Vertex(sf::Vector2f(position.x + left  - italicShear * top   , position.y + top),    color, sf::Vector2f(u1, v1)));
        vertices.append(sf::Vertex(sf::Vector2f(position.x + right - italicShear * top   , position.y + top),    color, sf::Vector2f(u2, v1)));
        vertices.append(sf::Vertex(sf::Vector2f(position.x + left  - italicShear * bottom, position.y + bottom), color, sf::Vector2f(u1, v2)));
        vertices.append(sf::Vertex(sf::Vector2f(position.x + left  - italicShear * bottom, position.y + bottom), color, sf::Vector2f(u1, v2)));
        vertices.append(sf::Vertex(sf::Vector2f(position.x + right - italicShear * top   , position.y + top),    color, sf::Vector2f(u2, v1)));
        vertices.append(sf::Vertex(sf::Vector2f(position.x + right - italicShear * bottom, position.y + bottom), color, sf::Vector2f(u2, v2)));
    }

}

sf::String GameText::getString(){
    return m_string;
}

int GameText::getRow(){
    return m_row;
}

void GameText::update(const sf::String character)
{
    if(character == m_string.substring(coloredIndex,1)){
        coloredIndex++;
    }else{
        coloredIndex = 0;
    }
}

bool GameText::isCompleted(){
    return m_string.getSize() == coloredIndex ? true : false;
}


GameText::GameText(sf::String value, const sf::Font& f, unsigned int characterSize):
    m_string             (value),
    m_font               (&f),
    m_characterSize      (characterSize),
    m_letterSpacingFactor(1.f),
    m_lineSpacingFactor  (1.f),
    m_fillColor          (255, 255, 255),
    m_vertices           (sf::Triangles),
    m_bounds             (),
    m_geometryNeedUpdate (true){}

GameText::GameText(sf::String value, const sf::Font& f, unsigned int characterSize, int row):
    m_string             (value),
    m_font               (&f),
    m_characterSize      (characterSize),
    m_letterSpacingFactor(1.f),
    m_lineSpacingFactor  (1.f),
    m_fillColor          (255, 255, 255),
    m_vertices           (sf::Triangles),
    m_bounds             (),
    m_geometryNeedUpdate (true),
    m_row                (row){}

void GameText::draw(sf::RenderTarget& target, sf::RenderStates states) const{
    ensureGeometryUpdate();
    states.transform *= getTransform();
    states.texture = &m_font->getTexture(m_characterSize);

    target.draw(m_vertices, states);
}

void GameText::ensureGeometryUpdate() const
{
    if (!m_font)
        return;

    // Mark geometry as updated
    m_geometryNeedUpdate = false;

    // Clear the previous geometry
    m_vertices.clear();

    // No text: nothing to draw
    if (m_string.isEmpty())
        return;

    float underlineOffset    = m_font->getUnderlinePosition(m_characterSize);
    float underlineThickness = m_font->getUnderlineThickness(m_characterSize);


    // Precompute the variables needed by the algorithm
    float whitespaceWidth = m_font->getGlyph(L' ', m_characterSize, false).advance;
    float letterSpacing   = ( whitespaceWidth / 3.f ) * ( m_letterSpacingFactor - 1.f );
    whitespaceWidth      += letterSpacing;
    float lineSpacing     = m_font->getLineSpacing(m_characterSize) * m_lineSpacingFactor;
    float x               = 0.f;
    float y               = static_cast<float>(m_characterSize);

    // Create one quad for each character
    float minX = static_cast<float>(m_characterSize);
    float minY = static_cast<float>(m_characterSize);
    float maxX = 0.f;
    float maxY = 0.f;
    uint32_t prevChar = 0;

    for (std::size_t i = 0; i < m_string.getSize(); ++i)
    {
        uint32_t curChar = m_string[i];

        // Skip the \r char to avoid weird graphical issues
        if (curChar == L'\r')
            continue;

        // Apply the kerning offset
        x += m_font->getKerning(prevChar, curChar, m_characterSize, false);

        prevChar = curChar;

        // Handle special characters
        if ((curChar == L' ') || (curChar == L'\n') || (curChar == L'\t'))
        {
            // Update the current bounds (min coordinates)
            minX = std::min(minX, x);
            minY = std::min(minY, y);

            switch (curChar)
            {
                case L' ':  x += whitespaceWidth;     break;
                case L'\t': x += whitespaceWidth * 4; break;
                case L'\n': y += lineSpacing; x = 0;  break;
            }

            // Update the current bounds (max coordinates)
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);

            // Next glyph, no need to create a quad for whitespace
            continue;
        }

        // Extract the current glyph's description
        const sf::Glyph& glyph = m_font->getGlyph(curChar, m_characterSize, false);

        // Add the glyph to the vertices
        sf::Color textColor{coloredIndex>i ? sf::Color::Red : sf::Color::White};
        addGlyphQuad(m_vertices, sf::Vector2f(x, y), textColor, glyph, 0);

        // Update the current bounds
        float left   = glyph.bounds.left;
        float top    = glyph.bounds.top;
        float right  = glyph.bounds.left + glyph.bounds.width;
        float bottom = glyph.bounds.top + glyph.bounds.height;

        minX = std::min(minX, x + left - 0 * bottom);
        maxX = std::max(maxX, x + right - 0 * top);
        minY = std::min(minY, y + top);
        maxY = std::max(maxY, y + bottom);

        // Advance to the next character
        x += glyph.advance + letterSpacing;
    }

    // Update the bounding rectangle
    m_bounds.left = minX;
    m_bounds.top = minY;
    m_bounds.width = maxX - minX;
    m_bounds.height = maxY - minY;
}

