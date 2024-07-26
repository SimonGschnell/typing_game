#include <SFML/Graphics.hpp>
#include <iostream>
#include <exception>

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

    sf::Text text{};
    text.setFont(font);
    text.setString("hello world");
    text.setColor(sf::Color::Red);
    text.setCharacterSize(55);



    auto window = sf::RenderWindow{ { 300, 300 }, "Typing Game" };
    window.setFramerateLimit(144);

    // enlarge the sprite to fit the window size
    sf::Vector2u windowSize = window.getSize();
    sf::Vector2u textureSize = oldPaper.getSize();
    sprite.setScale((float) windowSize.x / textureSize.x, (float) windowSize.y / textureSize.y);

    float rotation{1};
    int rounds{0};

    while (window.isOpen())
    {
        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        window.clear();

        //drawing happens here
        window.draw(sprite);
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
        window.draw(avatarSprite);

        window.draw(text);

        window.display();
    }
}
