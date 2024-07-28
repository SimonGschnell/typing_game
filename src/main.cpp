#include <SFML/Graphics.hpp>
#include <iostream>
#include <exception>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string_view>
#include <cstdint>
#include <algorithm>

void loadRecourse(sf::Texture &t, std::string file) {
    if(!t.loadFromFile(file)){
        throw std::runtime_error("was not able to load file " + file);
    }
}
void loadRecourse(sf::Font &t, std::string file) {
    if(!t.loadFromFile(file)){
        throw std::runtime_error("was not able to load file " + file);
    }
}


class Subscriber{
    public:
        Subscriber() = default;
        virtual void update(sf::String) = 0;
};

class Publisher{
private:
    std::unordered_map<std::string, Subscriber*> m_subscribers{};
public:
    Publisher() = default;

    void subscribe(sf::String key, Subscriber* s){
        // store subscriber in the hashmap
        m_subscribers[key.toAnsiString()]=s;
    }

    void unsubscribe(std::string &key){
        // remove the subscriber from the hashmap
        m_subscribers.erase(key);
    }

    void notify_subscribers(sf::String value){
        // call the update function of every subscriber in the hashmap
        for(std::pair<const std::string, Subscriber*> &s : m_subscribers){
            s.second->update(value);
        }
    }

    auto getSubscribers(){
        return m_subscribers;
    }
};

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

class AnotherText : public sf::Drawable, public sf::Transformable{
private:
    const sf::Font* m_font{};
    sf::String m_string{};
    unsigned int m_characterSize{};
    mutable sf::VertexArray   m_vertices{sf::PrimitiveType::Triangles};
    mutable bool m_geometryNeedUpdate{true};
    float m_lineSpacingFactor{1.f};
    float m_letterSpacingFactor{1.f};
    sf::Color m_fillColor;
    mutable sf::FloatRect   m_bounds;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        ensureGeometryUpdate();
        states.transform *= getTransform();
        states.texture = &m_font->getTexture(m_characterSize);

        target.draw(m_vertices, states);
    }

    void ensureGeometryUpdate() const
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
        //test
        int i =0;
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
            sf::Color col{sf::Color::Red};
            if(i>1){
                col = sf::Color::White;
            }

            addGlyphQuad(m_vertices, sf::Vector2f(x, y), col, glyph, 0);

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

public:
    AnotherText(sf::String value, const sf::Font& f, unsigned int characterSize):
    m_string             (value),
    m_font               (&f),
    m_characterSize      (characterSize),
    m_letterSpacingFactor(1.f),
    m_lineSpacingFactor  (1.f),
    m_fillColor          (255, 255, 255),
    m_vertices           (sf::Triangles),
    m_bounds             (),
    m_geometryNeedUpdate (true){}

    void setCustomColor(const size_t &length, const sf::Color& color)
    {

        if (!m_geometryNeedUpdate)
        {
            std::cout << m_vertices.getVertexCount() << std::endl;
            for (std::size_t i = 0; i < length; ++i)
                m_vertices[i].color = color;

        }



    }

};

class GameText : public sf::Drawable, public Subscriber{

private:
    sf::Text m_coloredText{};
    sf::Text m_endingText{};
    sf::Vector2f m_pos{};

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        target.draw(m_coloredText);
        target.draw(m_endingText);
    }

    void updateEndingPosition(){
        sf::FloatRect coloredTextBounds{m_coloredText.getLocalBounds()};
        m_endingText.setPosition(m_coloredText.getPosition().x + coloredTextBounds.width + m_coloredText.getLetterSpacing(), m_coloredText.getPosition().y);
    }

public:
    GameText(sf::Font &f, std::string value, sf::Vector2f position): m_pos{position}{

        m_endingText.setFont(f);
        m_endingText.setString(value);

        m_coloredText.setFont(f);
        m_coloredText.setString("");
        m_coloredText.setFillColor(sf::Color::Red);

        m_coloredText.setPosition(position);
        updateEndingPosition();
    }

    void update(sf::String character){

        sf::String endingString{m_endingText.getString()};

        if(endingString.getSize() == 0) return;


        sf::String first {endingString.substring(0,1)};
        if(character == first){
            m_coloredText.setString(m_coloredText.getString() + first);
            m_endingText.setString(endingString.substring(1,endingString.getSize()));
        }else{
            m_endingText.setString(m_coloredText.getString() + endingString);
            m_coloredText.setString("");
        }

        if(m_endingText.getString().getSize() == 0) m_coloredText.setFillColor(sf::Color::Transparent);

        updateEndingPosition();
    }

    sf::String getString() const {
        return m_coloredText.getString() + m_endingText.getString();
    }

    void move(sf::Vector2f direction){
        m_coloredText.move(direction);
        m_endingText.move(direction);
    }

};

int main()
{
    //background
    sf::Texture oldPaper;
    loadRecourse(oldPaper,"./textures/oldPaper.png");
    oldPaper.setSmooth(true);

    sf::Sprite sprite;
    sprite.setTexture(oldPaper);

    // player
    sf::Texture avatarTexture;
    loadRecourse(avatarTexture,"./textures/guy.png");

    sf::Sprite avatarSprite;
    avatarSprite.setTexture(avatarTexture);
    sf::Vector2f originalAvatarScale{avatarSprite.getScale()};
    avatarSprite.setOrigin(avatarTexture.getSize().x/2,avatarTexture.getSize().y/2);

    // text
    sf::Font font{};
    loadRecourse(font,"./fonts/roboto.tff");

    sf::Glyph a = font.getGlyph(68,30,true);
    sf::Text text{};
    text.setFont(font);
    text.setString("hello world");
    text.setColor(sf::Color::Red);
    text.setCharacterSize(55);



    auto window = sf::RenderWindow{ { 300, 300 }, "Typing Game", sf::Style::Fullscreen | sf::Style::Resize | sf::Style::Close};
    window.setFramerateLimit(144);

    // enlarge the sprite to fit the window size
    sf::Vector2u windowSize = window.getSize();
    sf::Vector2u textureSize = oldPaper.getSize();
    sprite.setScale((float) windowSize.x / textureSize.x, (float) windowSize.y / textureSize.y);

    float rotation{1};
    int rounds{0};
    int index{0};

    sf::Clock clock;
    std::vector<sf::String> test_arr {"t","h","i","s","i","s","a","t","e","s","t"};

    GameText game_text1{font, "bird", {50,50}};
    GameText game_text2{font, "house", {50,100}};
    GameText game_text3{font, "bath", {50,150}};
    GameText game_text4{font, "pizza", {50,200}};
    GameText game_text5{font, "pineapple", {50,250}};

    Publisher pub;
    pub.subscribe(game_text1.getString(),&game_text1);
    pub.subscribe(game_text2.getString(),&game_text2);
    pub.subscribe(game_text3.getString(),&game_text3);
    pub.subscribe(game_text4.getString(),&game_text4);
    pub.subscribe(game_text5.getString(),&game_text5);

    AnotherText t{"test",font,30};
    t.setCustomColor(10,sf::Color::Red);

    while (window.isOpen())
    {
        sf::Time elapsed = clock.getElapsedTime();

        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            switch(event.type){
                case sf::Event::Closed: window.close(); break;
                case sf::Event::TextEntered :
                    sf::String C{event.text.unicode};
                    pub.notify_subscribers(C);
                    break;
            }

        }

        window.clear();

        //drawing happens here
        /* window.draw(sprite);
        avatarSprite.move(sf::Vector2f(1.f,1.f));

        // do one 360 spin

        if(avatarSprite.getRotation() == 0.f){
            if(rounds > 0){
                rotation = 0.f;
            }
            rounds++;
        }

        if(avatarSprite.getScale().x < originalAvatarScale.x*1.5 && avatarSprite.getScale().y < originalAvatarScale.y*1.5 ){
            avatarSprite.scale(1.001f,1.001f);
        }

        avatarSprite.rotate(rotation);
        window.draw(avatarSprite); */

        window.draw(t);
        t.move({1.f,0});

        // here ------------------------------------------------------------------------
        /*
        for(auto pair : pub.getSubscribers()){
            GameText* t {static_cast<GameText*>(pair.second)};
            window.draw(*t);
            t->move({0.5f,0.f});
        } */

        /* if(elapsed.asSeconds() >= 1.f){
            try{
                game_text.advance(test_arr.at(index));
            }catch(std::out_of_range exception){
                std::cout << exception.what() << std::endl;
            }
            std::cout << "3 second passed" << std::endl;
            clock.restart();
            index++;
        } */


        window.display();
    }
}
