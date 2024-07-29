#include <SFML/Graphics.hpp>
#include <iostream>
#include <exception>
#include <vector>
#include <memory>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include <cstdlib>

#include "../include/Observer.hpp"
#include "../include/GameText.hpp"
#include "../include/httpRequest.hpp"


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
    // fetching pokemon names
    //std::string test{PokeApi::getPokemon(PokeApi::generatePokemonID()).first};

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
    GameText game_text1{"bird", font, 30};
    game_text1.setPosition({50,50});
    GameText game_text2{"house", font, 30};
    game_text2.setPosition({50,100});
    GameText game_text3{"bath", font, 30};
    game_text3.setPosition({50,150});
    GameText game_text4{"pizza", font, 30};
    game_text4.setPosition({50,200});
    GameText game_text5{"pineapple", font, 30};
    game_text5.setPosition({50,250});

    Publisher pub;
    pub.subscribe(game_text1.getString(),&game_text1);
    pub.subscribe(game_text2.getString(),&game_text2);
    pub.subscribe(game_text3.getString(),&game_text3);
    pub.subscribe(game_text4.getString(),&game_text4);
    pub.subscribe(game_text5.getString(),&game_text5);

    GameText game_test{test, font, 30};
    game_test.setPosition({50,350});
    pub.subscribe(game_test.getString(),&game_test);


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


        for(auto pair : pub.getSubscribers()){
            GameText* t {static_cast<GameText*>(pair.second)};

            if(t->isCompleted()){
                pub.unsubscribe(t->getString().toAnsiString());
            }else{
                window.draw(*t);
                t->move({0.5f,0.f});
            }
        }

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
