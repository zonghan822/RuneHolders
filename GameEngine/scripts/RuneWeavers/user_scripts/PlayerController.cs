using System;
using System.Collections.Generic;

namespace Eno
{
    public class PlayerController : ScriptBase
    {
        // Movement speed in world units per second
        public float speed = 300f;

        public float attackDuration = 0.5f;
        public float castDuration   = 0.5f;
        public float hurtDuration   = 0.4f;

        private Actor     _player = null!;
        private Canvas?   _canvas       = null;
        private Sprite2D? _hpBarFull;
        private bool      _hpBarReady   = false;
        private float     _hpBarInitW   = 0f;
        private float     _hpBarInitH   = 0f;
        private float     _hpBarLocalX  = 0f;
        private float     _hpBarLocalY  = 0f;
        private float     _attackTimer  = 0f;
        private float     _hurtTimer   = 0f;
        private bool      _isAttacking = false;
        private bool      _isHurt      = false;

        public float footstepVolume = 70f;

        private Node2D?[] _scrollNodes    = Array.Empty<Node2D?>();
        private uint      _spellbookUIID    = 0;
        private bool      _spellbookOpen    = false;
        private AudioNode? _spellbookAudio  = null;
        private AudioNode? _footstepAudio   = null;
        private Random     _rng             = new Random();
        private float      _footstepTimer   = 0f;
        private const float FOOTSTEP_INTERVAL = 0.375f;
        private uint      _winLoseCanvasID  = 0;
        private uint      _enemyInfoID        = 0;
        private uint      _enemyInfoPopupID   = 0;
        private uint      _enemyInfoPopupBgID = 0;
        private bool      _enemyInfoOpen      = false;
        private uint      _defeatScreenID     = 0;
        private float     _deathTimer         = 0f;
        private float     _deathDuration      = 1.0f;

        // In-game pause flag
        public static bool IsGamePaused = false;
        public static bool IsGameCompleted = false;

        // Canvas interface
        //private bool _UIPopUp = false;

        // Player script reference
        private static PlayerController? _instance;


        public void Begin()
        {
            _player = Self<Actor>()!;
            _instance = this;
            
            // ACTOR nodes have physics — keep gravity off for top-down movement
            _player.useGravity      = false;
            _player.isKinematic     = false;
            _player.collisionGroup  = 1;
            _player.maxHealth       = 5f;
            _player.health      = 5f;
            _player.damage      = 1f;

            
            _hpBarFull = GetNode<Sprite2D>("PlayerHealthBar_Full");
            try { _canvas = GetNode<Canvas>("Gameplay_UI"); } catch { }

            _scrollNodes = new Node2D?[Scroll_Dialouge_Script.ScrollNames.Length];
            for (int i = 0; i < _scrollNodes.Length; i++)
                try { _scrollNodes[i] = GetNode<Node2D>(Scroll_Dialouge_Script.ScrollNames[i]); } catch { }

            _spellbookAudio  = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;

            _footstepAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
            if (_footstepAudio != null)
            {
                _footstepAudio.volume = footstepVolume;
                _footstepAudio.spatialAudio = true;
            }

            _spellbookUIID   = Bridge.GetNode("Spellbook");
            _winLoseCanvasID = Bridge.GetNode("WinLoseCanvas");
            _enemyInfoID        = Bridge.GetNode("EnemyInfo");
            _enemyInfoPopupID   = Bridge.GetNode("EnemyInfoPopUp");
            _enemyInfoPopupBgID = Bridge.GetNode("EnemyInfoPopUpBG");
            _defeatScreenID     = Bridge.GetNode("DefeatScreen");

            IsGamePaused = false;
            IsGameCompleted = false;
        }

        private void UpdateHealthBar()
        {
            if (_hpBarFull == null) return;

            if (!_hpBarReady)
            {
                float w = _hpBarFull.width;
                if (w <= 0f) return;
                _hpBarInitW = w;
                _hpBarInitH = _hpBarFull.height;

                // Capture initial canvas-local offset so we can restore it each frame
                if (_canvas != null)
                {
                    _hpBarLocalX = _hpBarFull.position.x - Bridge.GetPositionX(_canvas.ID);
                    _hpBarLocalY = _hpBarFull.position.y - Bridge.GetPositionY(_canvas.ID);
                }

                _hpBarReady = true;
            }

            float pct  = _player.maxHealth > 0f ? _player.health / _player.maxHealth : 0f;
            float newW = _hpBarInitW * pct;

            // Shift the bar center left so the left edge stays fixed while the right edge shrinks
            float offsetX = (_hpBarInitW - newW) * 0.5f;
            Bridge.SetLocalPosition(_hpBarFull.ID, _hpBarLocalX - offsetX, _hpBarLocalY);
            _hpBarFull.setSize(newW, _hpBarInitH);
        }

        public void Update()
        {
            if (_player.health <= 0f)
                _player.changeState("Death");

            // P key or Start button toggles in-game pause
            if ((Input.isKeyPressed(KeyCode.P) || Input.isGamepadButtonPressed(GamepadButton.Start)) && !IsGameCompleted)
            {
                IsGamePaused = !IsGamePaused;
                Bridge.SetCanvasActive(_winLoseCanvasID, IsGamePaused);
                uint buttonID = Bridge.GetNode("PauseScreen");
                Bridge.SetVisibility(buttonID, IsGamePaused);
                SetChildButtonsActive(buttonID, IsGamePaused);
                Bridge.SetAudioPaused(IsGamePaused);
            }
            
            
            // E key or A/X button — close enemy info panel if open
            if (IsGamePaused && _enemyInfoOpen && (Input.isKeyPressed(KeyCode.E) || Input.isGamepadButtonPressed(GamepadButton.A) || Input.isGamepadButtonPressed(GamepadButton.X)))
            {
                _enemyInfoOpen = false;
                if (_enemyInfoID      != 0) Bridge.SetVisibility(_enemyInfoID, false);
                if (_enemyInfoPopupID != 0)
                {
                    Bridge.SetVisibility(_enemyInfoPopupID, false);
                    Bridge.SetButtonActive(_enemyInfoPopupID, false);
                }
                if (_enemyInfoPopupBgID != 0) Bridge.SetVisibility(_enemyInfoPopupBgID, false);
                IsGamePaused = false;
                Bridge.SetAudioPaused(false);
                return;
            }
            
            // F key or Y button — toggle spellbook UI (checked before pause guard so it can close while paused)
            if ((Input.isKeyPressed(KeyCode.F) || Input.isGamepadButtonPressed(GamepadButton.Y)) && _spellbookUIID != 0)
            {
                _spellbookOpen = !_spellbookOpen;
                Bridge.SetVisibility(_spellbookUIID, _spellbookOpen);
                IsGamePaused = _spellbookOpen;
                Bridge.SetAudioPaused(_spellbookOpen);
            }

            if (IsGamePaused) return;

            UpdateHealthBar();

            // Freeze player and skip input during dialogue
            if (Scroll_Dialouge_Script.DialogueActive)
            {
                _player.velocity = new Vector2(0f, 0f);
                return;
            }

            if (_isAttacking || _isHurt) return;

            float dt = Time.deltaTime;

            // Build direction vector from held keys
            float dx = 0f;
            float dy = 0f;

            if (Input.isKeyHeld(KeyCode.W)) dy -= 1f;
            if (Input.isKeyHeld(KeyCode.S)) dy += 1f;
            if (Input.isKeyHeld(KeyCode.A)) dx -= 1f;
            if (Input.isKeyHeld(KeyCode.D)) dx += 1f;

            // Merge left analog stick input
            Vector2 leftStick = Input.getLeftStick(0.2f);
            if (leftStick.x != 0f || leftStick.y != 0f)
            {
                dx = leftStick.x;
                dy = leftStick.y;
            }

            // Normalize diagonal movement so speed is consistent in all directions
            float mag = MathF.Sqrt(dx * dx + dy * dy);
            if (mag > 1f)
            {
                dx /= mag;
                dy /= mag;
            }


            // X button — open enemy info popup
            if (!_enemyInfoOpen && Input.isGamepadButtonPressed(GamepadButton.X))
            {
                _when_enemyinfopopup_onclick();
                return;
            }

            // Check scroll interaction before applying movement
            if (Input.isKeyPressed(KeyCode.E) || Input.isGamepadButtonPressed(GamepadButton.A))
            {
                for (int i = 0; i < _scrollNodes.Length; i++)
                {
                    if (_scrollNodes[i] != null && Bridge.IsColliding(_scrollNodes[i]!.ID, _player.ID))
                    {
                        Scroll_Dialouge_Script.TriggerDialogue(i);
                        return;
                    }
                }
            }

            _player.velocity = new Vector2(dx * speed, dy * speed);

            // Flip sprite horizontally based on horizontal direction
            if (dx != 0f)
            {
                Vector2 s = _player.scale;
                _player.scale = new Vector2(dx > 0f ? -MathF.Abs(s.x) : MathF.Abs(s.x), s.y);
            }
        }
    
        // Called when entering the idle state
        public void Idle_enter()
        {
            _player.setAnimationLooping(true);
        }

        // Called every frame while in the idle state
        public void Idle_update()
        {
            if (IsMoving())
                _player.changeState("Walk");
        }

        // Called when exiting the idle state
        public void Idle_exit()
        {
            // TODO: Add exit logic for idle state
        }

        // Called when entering the walk state
        public void Walk_enter()
        {
            _player.setAnimationLooping(true);
            _footstepTimer = FOOTSTEP_INTERVAL; // fire immediately on first step
            PlayFootstep();
        }

        // Called every frame while in the walk state
        public void Walk_update()
        {
            if (!IsMoving())
            {
                _player.changeState("Idle");
                return;
            }

            _footstepTimer += Time.deltaTime;
            if (_footstepTimer >= FOOTSTEP_INTERVAL)
            {
                _footstepTimer = 0f;
                PlayFootstep();
            }
        }

        // Called when exiting the walk state
        public void Walk_exit()
        {
            _footstepAudio?.stop();
            _footstepTimer = 0f;
        }

        private void SetChildButtonsActive(uint parentID, bool active)
        {
            uint count = Bridge.GetChildCount(parentID);
            for (uint i = 0; i < count; i++)
            {
                uint childID = Bridge.GetChild(parentID, i);
                if (Bridge.GetNodeType(childID) == (uint)NodeType.BUTTON)
                    Bridge.SetButtonActive(childID, active);
                SetChildButtonsActive(childID, active);
            }
        }

        private void PlayFootstep()
        {
            if (_footstepAudio == null) return;
            _footstepAudio.sound = "Footsteps-" + _rng.Next(1, 9);
            _footstepAudio.spatialPlay(_footstepAudio.ID, _player.position.x, _player.position.y);
        }

        private bool IsMoving()
        {
            if (Input.isKeyHeld(KeyCode.W) || Input.isKeyHeld(KeyCode.S) ||
                Input.isKeyHeld(KeyCode.A) || Input.isKeyHeld(KeyCode.D))
                return true;

            Vector2 stick = Input.getLeftStick(0.2f);
            return stick.x != 0f || stick.y != 0f;
        }
    
        public void Cast_enter()
        {
            _player.animation = "Cast";
            _player.setAnimationLooping(false);
            uint frames = _player.getAnimationFrameCount();
            if (frames > 0) _player.setAnimationFrameDuration(castDuration / frames);
        }

        public void Cast_update()
        {
            if (IsMoving())
                _player.changeState("CastWalk");
        }

        public void Cast_exit()
        {
            _player.setAnimationLooping(true);
        }
    
        public void CastWalk_enter()
        {
            _player.setAnimationLooping(true);
            _footstepTimer = FOOTSTEP_INTERVAL; // fire immediately on first step
            PlayFootstep();
        }

        public void CastWalk_update()
        {
            if (!IsMoving())
            {
                _player.changeState("Cast");
                return;
            }

            _footstepTimer += Time.deltaTime;
            if (_footstepTimer >= FOOTSTEP_INTERVAL)
            {
                _footstepTimer = 0f;
                PlayFootstep();
            }
        }

        public void CastWalk_exit()
        {
            _footstepAudio?.stop();
            _footstepTimer = 0f;
        }
    
        public void Attack_enter()
        {
            _isAttacking = true;
            _attackTimer = 0f;
            _player.velocity = new Vector2(0f, 0f);
            _player.animation = "Attack";
            _player.setAnimationLooping(false);
            uint frames = _player.getAnimationFrameCount();
            if (frames > 0) _player.setAnimationFrameDuration(attackDuration / frames);
        }

        public void Attack_update()
        {
            _attackTimer += Time.deltaTime;
            if (_attackTimer >= attackDuration)
                _player.changeState("Idle");
        }

        public void Attack_exit()
        {
            _isAttacking = false;
        }
    
        public void Hurt_enter()
        {
            _isHurt    = true;
            _hurtTimer = 0f;
            _player.velocity  = new Vector2(0f, 0f);
            _player.setAnimationLooping(false);
            uint frames = _player.getAnimationFrameCount();
            if (frames > 0) _player.setAnimationFrameDuration(hurtDuration / frames);
            Input.setGamepadRumble(0.5f, 0.5f);
        }

        public void Hurt_update()
        {
            _hurtTimer += Time.deltaTime;
            if (_hurtTimer >= hurtDuration)
                _player.changeState(IsMoving() ? "Walk" : "Idle");
            
        }

        public void Hurt_exit()
        {
            _isHurt = false;
            Input.setGamepadRumble(0f, 0f);
            _player.setAnimationLooping(true);
        }
    
        public void Death_enter()
        {
            _deathTimer = 0f;
            _player.stateChangeLocked = true;
            _player.velocity = new Vector2(0f, 0f);
            _player.setAnimationLooping(false);
            uint frames = _player.getAnimationFrameCount();
            if (frames > 0) _player.setAnimationFrameDuration(_deathDuration / frames);
            Input.setGamepadRumble(1.0f, 1.0f);
        }

        public void Death_update()
        {
            _player.velocity = new Vector2(0f, 0f);
            _player.setAnimationLooping(false);
            _deathTimer += Time.deltaTime;
            if (_deathTimer >= _deathDuration)
            {
                Input.setGamepadRumble(0f, 0f);
                if (_winLoseCanvasID != 0)
                {
                    Bridge.SetCanvasActive(_winLoseCanvasID, true);
                    if (_defeatScreenID != 0)
                    {
                        Bridge.SetVisibility(_defeatScreenID, true);
                        SetChildButtonsActive(_defeatScreenID, true);
                    }
                }
            }
        }

        public void Death_exit()
        {
        }
    
		public void _when_enemyinfopopup_onclick()
        {
            if (_enemyInfoID != 0)
            {
                _enemyInfoOpen = true;
                Bridge.SetVisibility(_enemyInfoID, true);
                IsGamePaused = true;
                Bridge.SetAudioPaused(true);
            }
        }
    }
}
