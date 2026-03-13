namespace Eno
{
    // Shared combat utility for enemies.
    // Any enemy script can call these to deal damage to the player
    // without duplicating i-frame or health logic.
    public static class EnemyCombat
    {
        // Deal contact damage to the player as a percentage of their max health.
        // damagePercent: 0.0–1.0  (0.2 = 20% max HP)
        // iFrameDuration: seconds the player cannot be hit again after this hit
        // Returns true if the hit was applied (false if player was invulnerable or dead).
        public static bool TryDealContactDamage(uint playerID, float damagePercent, float iFrameDuration)
        {
            if (Bridge.IsDead(playerID) || Bridge.IsInvulnerable(playerID))
                return false;

            float damage = Bridge.GetMaxHealth(playerID) * damagePercent;
            Bridge.TakeDamageFlat(playerID, damage);
            Bridge.SetInvulnerable(playerID, true);
            Bridge.SetInvulnerabilityTimer(playerID, iFrameDuration);
            return true;
        }

        // Tick down the player's i-frame timer. Call this from PlayerController.Update().
        public static void TickInvulnerability(uint playerID, float dt)
        {
            if (!Bridge.IsInvulnerable(playerID)) return;

            float remaining = Bridge.GetInvulnerabilityTimer(playerID) - dt;
            if (remaining <= 0f)
            {
                Bridge.SetInvulnerabilityTimer(playerID, 0f);
                Bridge.SetInvulnerable(playerID, false);
            }
            else
            {
                Bridge.SetInvulnerabilityTimer(playerID, remaining);
            }
        }
    }
}
