#include "input.h"
#include "engine.h"

P<WindowManager> InputHandler::windowManager;
bool InputHandler::touch_screen = false;
sf::Transform InputHandler::mouse_transform;
PVector<InputEventHandler> InputHandler::input_event_handlers;
PVector<JoystickEventHandler> InputHandler::joystick_event_handlers;

bool InputHandler::keyboard_button_down[sf::Keyboard::KeyCount];
bool InputHandler::keyboard_button_pressed[sf::Keyboard::KeyCount];
bool InputHandler::keyboard_button_released[sf::Keyboard::KeyCount];
sf::Event::KeyEvent InputHandler::last_key_press;

sf::Vector2f InputHandler::mouse_position;
float InputHandler::mouse_wheel_delta;
bool InputHandler::mouse_button_down[sf::Mouse::ButtonCount];
bool InputHandler::mouse_button_pressed[sf::Mouse::ButtonCount];
bool InputHandler::mouse_button_released[sf::Mouse::ButtonCount];

float InputHandler::joystick_axis_pos[sf::Joystick::AxisCount];
float InputHandler::joystick_axis_changed[sf::Joystick::AxisCount];
bool InputHandler::joystick_button_down[sf::Joystick::ButtonCount];
bool InputHandler::joystick_button_changed[sf::Joystick::ButtonCount];

InputEventHandler::InputEventHandler()
{
    InputHandler::input_event_handlers.push_back(this);
}

InputEventHandler::~InputEventHandler()
{
}

JoystickEventHandler::JoystickEventHandler()
{
    InputHandler::joystick_event_handlers.push_back(this);
}

JoystickEventHandler::~JoystickEventHandler()
{
}

void InputHandler::initialize()
{
    memset(mouse_button_down, 0, sizeof(mouse_button_down));
    memset(keyboard_button_down, 0, sizeof(keyboard_button_down));
    memset(joystick_axis_pos, 0, sizeof(joystick_axis_pos));
#ifdef __ANDROID__
    touch_screen = true;
#endif
    last_key_press.code = sf::Keyboard::Unknown;
}

void InputHandler::preEventsUpdate()
{
    if (!windowManager)
        windowManager = engine->getObject("windowManager");

    for(unsigned int n=0; n<sf::Keyboard::KeyCount; n++)
    {
        if (keyboard_button_pressed[n])
            keyboard_button_pressed[n] = false;
        else
            keyboard_button_released[n] = false;
    }
    for(unsigned int n=0; n<sf::Mouse::ButtonCount; n++)
    {
        if (mouse_button_pressed[n])
            mouse_button_pressed[n] = false;
        else
            mouse_button_released[n] = false;
    }
    for(unsigned int n=0; n<sf::Joystick::AxisCount; n++)
    {
        joystick_axis_changed[n] = false;
    }
    for(unsigned int n=0; n<sf::Joystick::ButtonCount; n++)
    {
        joystick_button_changed[n] = false;
    }
    mouse_wheel_delta = 0;
}

void InputHandler::handleEvent(sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code > sf::Keyboard::Unknown && event.key.code < sf::Keyboard::KeyCount)
        {
            keyboard_button_down[event.key.code] = true;
            keyboard_button_pressed[event.key.code] = true;
        }
        last_key_press = event.key;
    }
	else if (event.type == sf::Event::KeyReleased)
	{
        if (event.key.code > sf::Keyboard::Unknown && event.key.code < sf::Keyboard::KeyCount)
        {
            keyboard_button_down[event.key.code] = false;
            keyboard_button_released[event.key.code] = true;
        }
	}
    else if (event.type == sf::Event::TextEntered && event.text.unicode > 31 && event.text.unicode < 128)
    {
        ////if (last_key_press.code != sf::Keyboard::Unknown)
        ////{
            fireKeyEvent(last_key_press, event.text.unicode);
            last_key_press.code = sf::Keyboard::Unknown;
        ////}
    }
    else if (event.type == sf::Event::MouseWheelMoved)
        mouse_wheel_delta += event.mouseWheel.delta;
    if (event.type == sf::Event::MouseButtonPressed)
    {
        mouse_button_down[event.mouseButton.button] = true;
        mouse_button_pressed[event.mouseButton.button] = true;
    }
    else if (event.type == sf::Event::MouseButtonReleased)
    {
        mouse_button_down[event.mouseButton.button] = false;
        mouse_button_released[event.mouseButton.button] = true;
    }
    else if (event.type == sf::Event::JoystickMoved && event.joystickMove.joystickId == 0)
    {
        float axis_pos;
        if (event.joystickMove.position > joystick_axis_snap_to_0_range) {
            axis_pos = (event.joystickMove.position - joystick_axis_snap_to_0_range) * ((joystick_axis_snap_to_0_range / 100) + 1);
        } else if (event.joystickMove.position < -joystick_axis_snap_to_0_range) {
            axis_pos = (event.joystickMove.position + joystick_axis_snap_to_0_range) * ((joystick_axis_snap_to_0_range / 100) + 1);
        } else {
            axis_pos = 0.0;
        }
        if (joystick_axis_pos[event.joystickMove.axis] != axis_pos){
            joystick_axis_changed[event.joystickMove.axis] = true;
        }
        joystick_axis_pos[event.joystickMove.axis] = axis_pos;
    }
    else if (event.type == sf::Event::JoystickButtonPressed && event.joystickButton.joystickId == 0)
    {
        joystick_button_down[event.joystickButton.button] = true;
        joystick_button_changed[event.joystickButton.button] = true;
    }
    else if (event.type == sf::Event::JoystickButtonReleased && event.joystickButton.joystickId == 0)
    {
        joystick_button_down[event.joystickButton.button] = false;
        joystick_button_changed[event.joystickButton.button] = true;
    }
}

void InputHandler::postEventsUpdate()
{
    if (last_key_press.code != sf::Keyboard::Unknown)
    {
        InputHandler::fireKeyEvent(last_key_press, -1);
        last_key_press.code = sf::Keyboard::Unknown;
    }

#ifdef __ANDROID__
    if (sf::Touch::isDown(0))
    {
        mouse_position = realWindowPosToVirtual(sf::Touch::getPosition(0));
        if (!mouse_button_down[sf::Mouse::Left])
            mouse_button_pressed[sf::Mouse::Left] = true;
        mouse_button_down[sf::Mouse::Left] = true;
    }else{
        if (mouse_button_down[sf::Mouse::Left])
            mouse_button_released[sf::Mouse::Left] = true;
        mouse_button_down[sf::Mouse::Left] = false;
    }
#else
    mouse_position = realWindowPosToVirtual(sf::Mouse::getPosition(windowManager->window));
#endif
    mouse_position = mouse_transform.transformPoint(mouse_position);
    
    if (touch_screen)
    {
        bool any_button_down = false;
        for(unsigned int n=0; n<sf::Mouse::ButtonCount; n++)
            if (mouse_button_down[n] || mouse_button_released[n])
                any_button_down = true;
        if (!any_button_down)
        {
            mouse_position = sf::Vector2f(-1, -1);
        }
    }
    for(unsigned int n=0; n<sf::Joystick::AxisCount; n++)
    {
        if(joystick_axis_changed[n])
        {
            foreach(JoystickEventHandler, e, joystick_event_handlers)
            {
                e->handleJoystickAxis(0, (sf::Joystick::Axis) n, joystick_axis_pos[n]);
            }
        }
    }
    for(unsigned int n=0; n<sf::Joystick::ButtonCount; n++)
    {
        if(joystick_button_changed[n])
        {
            foreach(JoystickEventHandler, e, joystick_event_handlers)
            {
                e->handleJoystickButton(0, n, joystick_button_down[n]);
            }
        }
    }
}

void InputHandler::setMousePos(sf::Vector2f position)
{
    if (!windowManager)
        windowManager = engine->getObject("windowManager");

    sf::Mouse::setPosition(virtualWindowPosToReal(position), windowManager->window);
    mouse_position = realWindowPosToVirtual(sf::Mouse::getPosition(windowManager->window));
}

void InputHandler::fireKeyEvent(sf::Event::KeyEvent key, int unicode)
{
    foreach(InputEventHandler, e, input_event_handlers)
    {
        e->handleKeyPress(key, unicode);
    }
}

sf::Vector2f InputHandler::realWindowPosToVirtual(sf::Vector2i position)
{
    sf::FloatRect viewport = windowManager->window.getView().getViewport();
    sf::Vector2f pos = sf::Vector2f(position);
    
    pos.x -= viewport.left * float(windowManager->window.getSize().x);
    pos.y -= viewport.top * float(windowManager->window.getSize().y);
    pos.x *= float(windowManager->virtualSize.x) / float(windowManager->window.getSize().x) / viewport.width;
    pos.y *= float(windowManager->virtualSize.y) / float(windowManager->window.getSize().y) / viewport.height;
    return pos;
}

sf::Vector2i InputHandler::virtualWindowPosToReal(sf::Vector2f position)
{
    sf::FloatRect viewport = windowManager->window.getView().getViewport();

    position.x /= float(windowManager->virtualSize.x) / float(windowManager->window.getSize().x) / viewport.width;
    position.y /= float(windowManager->virtualSize.y) / float(windowManager->window.getSize().y) / viewport.height;
    
    position.x += viewport.left * float(windowManager->window.getSize().x);
    position.y += viewport.top * float(windowManager->window.getSize().y);
    return sf::Vector2i(position);
}
