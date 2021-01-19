#include "bnAnimatedTextBox.h"
#include <cmath>

AnimatedTextBox::AnimatedTextBox(const sf::Vector2f& pos) : 
  textArea(), 
  totalTime(0), 
  textBox(280, 40) {
  textureRef = LOAD_TEXTURE(ANIMATED_TEXT_BOX);
  frame = sf::Sprite(*textureRef);

  // set the textbox positions
  //textBox.setPosition(sf::Vector2f(45.0f, 20.0f));
  setPosition(pos);
  setScale(2.0f, 2.0f);

  textSpeed = 2.0;

  // Load the textbox animation
  animator = Animation("resources/ui/textbox.animation");
  animator.Reload();

  isPaused = true;
  isReady = false;
  isOpening = false;
  isClosing = false;

  textBox.SetTextFillColor(sf::Color::Black);
}

AnimatedTextBox::~AnimatedTextBox() { }

void AnimatedTextBox::Close() {
  if (!isReady || isClosing) return;

  isClosing = true;
  isReady = false;

  animator.SetAnimation("CLOSE");

  auto callback = [this]() {
    isClosing = false;
    isOpening = false;
    isPaused = true;
  };

  animator << callback;
}


void AnimatedTextBox::Open(const std::function<void()>& onOpen) {
  if (isReady || isOpening) return;

  isOpening = true;

  animator.SetAnimation("OPEN");

  auto callback = [this, onOpen]() {
    isClosing = false;
    isPaused = false;
    isOpening = false;
    isReady = true;

    if (onOpen) onOpen();
  };

  animator << callback;
}

const bool AnimatedTextBox::IsPlaying() const { return !isPaused; }
const bool AnimatedTextBox::IsOpen() const { return isReady; }
const bool AnimatedTextBox::IsClosed() const { return !isReady && !isOpening && !isClosing; }

const bool AnimatedTextBox::HasMessage() {
  return (messages.size() > 0);
}

const bool AnimatedTextBox::IsEndOfMessage()
{
  return !textBox.HasMore() && IsEndOfBlock();
}

const bool AnimatedTextBox::IsEndOfBlock()
{
  return textBox.IsEndOfBlock();
}

void AnimatedTextBox::ShowNextLines()
{
  for (int i = 0; i < textBox.GetNumberOfFittingLines(); i++) {
    textBox.ShowNextLine();
  }

  isPaused = false;
}

const int AnimatedTextBox::GetNumberOfFittingLines() const
{
    return textBox.GetNumberOfFittingLines();
}

const float AnimatedTextBox::GetFrameWidth() const
{
  return frame.getLocalBounds().width;
}

const float AnimatedTextBox::GetFrameHeight() const
{
  return frame.getLocalBounds().height;
}

void AnimatedTextBox::CompleteCurrentBlock() {
  textBox.CompleteCurrentBlock();

  if (mugAnimator.GetAnimationString() != "IDLE") {
    mugAnimator.SetAnimation("IDLE");
    mugAnimator << Animator::Mode::Loop;
  }

  isPaused = false;
}

void AnimatedTextBox::DequeMessage() {
  if (messages.size() == 0) return;

  delete *messages.begin(); // TODO: use shared ptrs
  messages.erase(messages.begin());
  animPaths.erase(animPaths.begin());
  mugshots.erase(mugshots.begin());

  if (messages.size() == 0) return;

  mugAnimator = Animation(animPaths[0]);
  mugAnimator.SetAnimation("TALK");
  mugAnimator << Animator::Mode::Loop;
  textBox.SetText(messages[0]->GetMessage());

  isPaused = false; // Begin playing again
}

void AnimatedTextBox::ClearAllMessages()
{
  while (messages.size()) {
    DequeMessage();
  }
}

void AnimatedTextBox::EnqueMessage(sf::Sprite speaker, std::string animationPath, MessageInterface* message) {
  speaker.setScale(2.0f, 2.0f);
  messages.push_back(message);

  animPaths.push_back(animationPath);
  mugshots.push_back(speaker);

  mugAnimator = Animation(animPaths[0]);
  mugAnimator.SetAnimation("TALK");
  mugAnimator << Animator::Mode::Loop;

  std::string strMessage = messages[0]->GetMessage();
  textBox.SetText(strMessage);

  message->SetTextBox(this);
}

/*void AnimatedTextBox::ReplaceText(std::string text)
{
    textBox.SetText(text);
    isPaused = false; // start over with new text
}*/

void AnimatedTextBox::Update(double elapsed) {
  if (isReady && messages.size() > 0) {

    int yIndex = (int)(textBox.GetNumberOfLines() % textBox.GetNumberOfFittingLines());
    auto y = (textBox.GetNumberOfFittingLines() -yIndex) * 10.0f;
    y = frame.getPosition().y - y;

    if (!isPaused) {
      if (mugAnimator.GetAnimationString() != "TALK") {
        mugAnimator.SetAnimation("TALK");
        mugAnimator << Animator::Mode::Loop;
      }

      textBox.Update(elapsed*(float)textSpeed);

      if (textBox.IsEndOfMessage() || textBox.HasMore()) {
        isPaused = true;
      }
    }
    else {
      if (mugAnimator.GetAnimationString() != "IDLE") {
        mugAnimator.SetAnimation("IDLE");
        mugAnimator << Animator::Mode::Loop;
      }
    }

    mugAnimator.Update((float)(elapsed*textSpeed), mugshots.front());

    messages.front()->OnUpdate(elapsed);
  }

  textBox.Play(!isPaused);

  // set the textbox position
  textBox.setPosition(sf::Vector2f(getPosition().x + 100.0f, getPosition().y - 40.0f));

  animator.Update((float)elapsed, frame);
}

void AnimatedTextBox::SetTextSpeed(double factor) {
  if (textSpeed >= 1.0) {
    textSpeed = factor;
  }
}

void AnimatedTextBox::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
  frame.setScale(getScale());
  frame.setPosition(getPosition());
  frame.setRotation(getRotation());

  if (isOpening || isReady || isClosing) {
    target.draw(frame);
  }

  if (messages.size() > 0 && isReady) {
    sf::Sprite& sprite = mugshots.front();

    sf::Vector2f oldpos = sprite.getPosition();
    auto pos = oldpos;
    pos += getPosition();

    // This is a really bad design hack
    // I inherited the text area class which uses it's position as the text
    // starting point. But the textbox does NOT have text on the left-hand side
    // and it begins text offset to the right.
    // So we store the state of the object and then calculate the offset, and then
    // restore it when we draw
    // Prime example where scene nodes would come in handy.
    // TODO: Use SceneNodes now that we have them 12/6/2020

    pos += sf::Vector2f(6.0f, 2.0f - sprite.getGlobalBounds().height / 2.0f);

    sprite.setPosition(pos);

    mugAnimator.Update(0, sprite);

    if (IsOpen()) {
      target.draw(sprite);
      sprite.setPosition(oldpos);
    }

    //states.transform = getTransform();

    messages.front()->OnDraw(target, states);
  }
}

void AnimatedTextBox::DrawMessage(sf::RenderTarget & target, sf::RenderStates states) const
{
  target.draw(textBox, states);
}

Text AnimatedTextBox::MakeTextObject(const std::string& data)
{
  Text obj = textBox.GetText();
  obj.SetString(data);
  obj.setScale(2.f, 2.f);
  return obj;
}
