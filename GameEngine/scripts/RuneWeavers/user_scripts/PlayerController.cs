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
        private float     _hurtTimer    = 0f;
        private bool      _isAttacking  = false;
        private bool      _isHurt       = false;
        private float     _deathTimer   = 0f;
        private float     _deathDuration = 1.0f;

        public float footstepVolume = 70f;

        private Node2D?[]  _scrollNodes      = Array.Empty<Node2D?>();
        private AudioNode? _footstepAudio    = null;
        private AudioNode? _playerVoAudio    = null;
        private Random     _rng              = new Random();
        private float      _footstepTimer    = 0f;
        private const float FOOTSTEP_INTERVAL  = 0.375f;
        private const int   HURT_SOUND_COUNT   = 15;
        private const int   DEATH_SOUND_COUNT  = 2;
        private uint       _winLoseCanvasID  = 0;
        private uint       _defeatScreenID   = 0;

        // Forwards to PauseManager so other scripts can keep using PlayerController.IsGamePaused
        public static bool IsGamePaused => PauseManager.IsGamePaused;
        public static bool IsGameCompleted = false;
        public static bool SpellbookOpen = false;

        private static PlayerController? _instance;

        public void Begin()
        {
            _player = Self<Actor>()!;
            _instance = this;

            _player.useGravity     = false;
            _player.isKinematic    = false;
            _player.collisionGroup = 1;
            _player.maxHealth      = 5f;
            _player.health         = 5f;
            _player.damage         = 1f;

            _hpBarFull = GetNode<Sprite2D>("PlayerHealthBar_Full");
            try { _canvas = GetNode<Canvas>("Gameplay_UI"); } catch { }

            _scrollNodes = new Node2D?[Scroll_Dialouge_Script.ScrollNames.Length];
            for (int i = 0; i < _scrollNodes.Length; i++)
                try { _scrollNodes[i] = GetNode<Node2D>(Scroll_Dialouge_Script.ScrollNames[i]); } catch { }

            _footstepAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;
            if (_footstepAudio != null)
            {
                _footstepAudio.volume = footstepVolume;
                _footstepAudio.spatialAudio = true;
            }

            _playerVoAudio = SpawnPrefab("DefaultAudio", 0f, 0f) as AudioNode;

            _winLoseCanvasID = Bridge.GetNode("WinLoseCanvas");
            _defeatScreenID  = Bridge.GetNode("DefeatScreen");

            PauseManager.Reset();
            IsGameCompleted = false;
            SlugBase_Script.ResetGameEndState();
            BatBase_Script.ResetGameEndState();
            NullflyBase_Script.ResetGameEndState();
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

                if (_canvas != null)
                {
                    _hpBarLocalX = _hpBarFull.position.x - Bridge.GetPositionX(_canvas.ID);
                    _hpBarLocalY = _hpBarFull.position.y - Bridge.GetPositionY(_canvas.ID);
                }

                _hpBarReady = true;
            }

            float pct  = _player.maxHealth > 0f ? _player.health / _player.maxHealth : 0f;
            float newW = _hpBarInitW * pct;
            float offsetX = (_hpBarInitW - newW) * 0.5f;
            Bridge.SetLocalPosition(_hpBarFull.ID, _hpBarLocalX - offsetX, _hpBarLocalY);
            _hpBarFull.setSize(newW, _hpBarInitH);
        }

        public void Update()
        {
            if (_player.health <= 0f)
                _player.changeState("Death");

            if (IsGamePaused)
            {
                _player.velocity = new Vector2(0f, 0f);
                return;
            }

            UpdateHealthBar();

            if (Scroll_Dialouge_Script.DialogueActive)
            {
                _player.velocity = new Vector2(0f, 0f);
                return;
            }

            if (_isAttacking || _isHurt) return;

            bool isController = Input.inputMode == InputMode.Controller;

            float dx = 0f;
            float dy = 0f;

            if (Input.isKeyHeld(KeyCode.W)) dy -= 1f;
            if (Input.isKeyHeld(KeyCode.S)) dy += 1f;
            if (Input.isKeyHeld(KeyCode.A)) dx -= 1f;
            if (Input.isKeyHeld(KeyCode.D)) dx += 1f;

            if (isController)
            {
                Vector2 leftStick = Input.getLeftStick(0.2f);
                if (leftStick.x != 0f || leftStick.y != 0f)
                {
                    dx = leftStick.x;
                    dy = leftStick.y;
                }
            }

            float mag = MathF.Sqrt(dx * dx + dy * dy);
            if (mag > 1f) { dx /= mag; dy /= mag; }

            // Check scroll interaction before applying movement
            if (Input.isKeyPressed(KeyCode.E) || (isController && Input.isGamepadButtonPressed(GamepadButton.A)))
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

            if (dx != 0f)
            {
                Vector2 s = _player.scale;
                _player.scale = new Vector2(dx > 0f ? -MathF.Abs(s.x) : MathF.Abs(s.x), s.y);
            }
        }

        public void Idle_enter()  { _player.setAnimationLooping(true); }
        public void Idle_exit()   { }

        public void Idle_update()
        {
            if (IsGamePaused || Scroll_Dialouge_Script.DialogueActive) return;
            if (IsMoving()) _player.changeState("Walk");
        }

        public void Walk_enter()
        {
            _player.setAnimationLooping(true);
            _footstepTimer = FOOTSTEP_INTERVAL; // fire immediately on first step
            PlayFootstep();
        }

        public void Walk_update()
        {
            if (IsGamePaused || Scroll_Dialouge_Script.DialogueActive)
            {
                _player.changeState("Idle");
                return;
            }
            if (!IsMoving()) { _player.changeState("Idle"); return; }

            _footstepTimer += Time.deltaTime;
            if (_footstepTimer >= FOOTSTEP_INTERVAL)
            {
                _footstepTimer = 0f;
                PlayFootstep();
            }
        }

        public void Walk_exit()
        {
            _footstepAudio?.stop();
            _footstepTimer = 0f;
        }

        public void Cast_enter()
        {
            _player.animation = "Cast";
            _player.setAnimationLooping(false);
            uint frames = _player.getAnimationFrameCount();
            if (frames > 0) _player.setAnimationFrameDuration(castDuration / frames);
        }

        public void Cast_update()  { if (IsMoving()) _player.changeState("CastWalk"); }
        public void Cast_exit()    { _player.setAnimationLooping(true); }

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
            if (_attackTimer >= attackDuration) _player.changeState("Idle");
        }

        public void Attack_exit() { _isAttacking = false; }

        public void Hurt_enter()
        {
            _isHurt    = true;
            _hurtTimer = 0f;
            _player.velocity = new Vector2(0f, 0f);
            _player.setAnimationLooping(false);
            uint frames = _player.getAnimationFrameCount();
            if (frames > 0) _player.setAnimationFrameDuration(hurtDuration / frames);
            Input.setGamepadRumble(0.5f, 0.5f);
            if (_playerVoAudio != null)
            {
                _playerVoAudio.sound = "Player_VO_Hurt_" + _rng.Next(1, HURT_SOUND_COUNT + 1).ToString("D2");
                _playerVoAudio.play();
            }
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
            if (_playerVoAudio != null)
            {
                _playerVoAudio.sound = "Player_Death_TEMP_" + _rng.Next(1, DEATH_SOUND_COUNT + 1).ToString("D2");
                _playerVoAudio.play();
            }
        }

        public void Death_update()
        {
            _player.velocity = new Vector2(0f, 0f);
            _player.setAnimationLooping(false);
            _deathTimer += Time.deltaTime;
            if (_deathTimer >= _deathDuration)
            {
                Input.setGamepadRumble(0f, 0f);
                IsGameCompleted = true;
                BGMforLevel1_Script.StopAllBGM();
                SlugBase_Script.StopAllWalkAudio();
                NullflyBase_Script.StopAllAudio();
                BatBase_Script.StopAllAudio();
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

        public void Death_exit() { }

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

            if (Input.inputMode == InputMode.Controller)
            {
                Vector2 stick = Input.getLeftStick(0.2f);
                return stick.x != 0f || stick.y != 0f;
            }
            return false;
        }
    }
}
