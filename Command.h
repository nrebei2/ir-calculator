enum Command
{
  Zero = 48,
  One = 49,
  Two = 50,
  Three = 51,
  Four = 52,
  Five = 53,
  Six = 54,
  Seven = 55,
  Eight = 56,
  Nine = 57,
  Enter,
  Up,
  Down,
  Left,
  Right,
  Plus,
  Minus,
  Unused
};

Command fromIRReading(unsigned long val)
{
  switch (val)
  {
  case 0xFF6897:
    return Zero;
  case 0xFF30CF:
    return One;
  case 0xFF18E7:
    return Two;
  case 0xFF7A85:
    return Three;
  case 0xFF10EF:
    return Four;
  case 0xFF38C7:
    return Five;
  case 0xFF5AA5:
    return Six;
  case 0xFF42BD:
    return Seven;
  case 0xFF4AB5:
    return Eight;
  case 0xFF52AD:
    return Nine;
  case 0xFFE01F:
    return Down;
  case 0xFF906F:
    return Up;
  case 0xFF9867:
    return Enter;
  case 0xFF22DD:
    return Left;
  case 0xFFC23D:
    return Right;
  case 0xFF629D:
    return Plus;
  case 0xFFA857:
    return Minus;
  default:
    return Unused;
  }
}