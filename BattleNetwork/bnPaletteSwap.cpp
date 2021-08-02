#include "bnPaletteSwap.h"
#include "bnTextureResourceManager.h"
#include "bnShaderResourceManager.h"
#include "bnEntity.h"
#include "bnCharacter.h"

PaletteSwap::PaletteSwap(Entity * owner) : Component(owner), enabled(true)
{
  paletteSwap = ResourceHandle().Shaders().GetShader(ShaderType::PALETTE_SWAP);
  paletteSwap.SetUniform("texture", sf::Shader::CurrentTexture);
  asCharacter = dynamic_cast<Character*>(owner);
}

PaletteSwap::~PaletteSwap()
{
}

void PaletteSwap::OnUpdate(double _elapsed)
{
  if (!enabled) return;
  if (GetOwner()->GetShader().Get() == paletteSwap.Get()) return;
  if (asCharacter && asCharacter->IsStunned()) return;
  
  GetOwner()->SetShader(paletteSwap);
}

void PaletteSwap::Inject(BattleSceneBase&)
{
}

void PaletteSwap::LoadPaletteTexture(std::string path)
{
  palette = ResourceHandle().Textures().LoadTextureFromFile(path);
  paletteSwap.SetUniform("palette", *palette);

}

void PaletteSwap::CopyFrom(PaletteSwap& other)
{
  enabled = other.enabled;
  base = other.base;
  palette = other.palette;
  Apply();
}

void PaletteSwap::SetPalette(const std::shared_ptr<sf::Texture>& palette)
{
  this->palette = palette;
  paletteSwap.SetUniform("palette", *palette);
}

void PaletteSwap::SetBase(const std::shared_ptr<sf::Texture>& base)
{
  this->base = base;
  paletteSwap.SetUniform("palette", *base);
}

void PaletteSwap::Revert()
{
  SetPalette(base);
}

void PaletteSwap::Enable(bool enabled)
{
  PaletteSwap::enabled = enabled;

  // Apply it on this call
  // Don't wait for the next frame (update())
  // Otherwise blocky effects occur
  if (enabled) {
    Apply();
  }
  else {
    GetOwner()->SetShader(nullptr);
  }
}

void PaletteSwap::Apply()
{
  GetOwner()->SetShader(paletteSwap);
}

const bool PaletteSwap::IsEnabled() const
{
  return enabled;
}
