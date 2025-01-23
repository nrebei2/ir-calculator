#include "History.h"
class Calculator
{
public:
  static const u16 LINE_SIZE = 30;

private:
  History history;

  char line_buffer[Calculator::LINE_SIZE + 1] = {0}; // extra \0 at end so parsing functions dont run off
  u8 line_size;

  // State
  enum StateType
  {
    Select,
    Insert
  };

  struct SelectData
  {
    u8 history_cursor;
    bool selecting_question; // otherwise selecting answer
  };

  struct InsertData
  {
    u8 input_cursor;
    u8 print_offset; // cursor should lie on [print_offset, print_offset + 16]
  };

  union State
  {
    SelectData on_select;
    InsertData on_insert;
  };

  StateType state_type;
  State state;

public:
  Calculator()
  {
    this->line_size = 0;
    this->state_type = Insert;
    this->state.on_insert = {0, 0};
  }

  void handleDigit(char digit)
  {
    if (this->line_size == Calculator::LINE_SIZE)
    {
      return;
    }

    if (this->state_type == Insert)
    {
      auto place_ptr = this->line_buffer + this->state.on_insert.input_cursor;

      // shift and place
      memmove(place_ptr + 1, place_ptr, this->line_size - this->state.on_insert.input_cursor);
      *place_ptr = digit;
      this->state.on_insert.input_cursor++;
      this->line_size++;

      if (this->state.on_insert.input_cursor >= this->state.on_insert.print_offset + SCREEN_WIDTH)
      {
        // move a unit right
        this->state.on_insert.print_offset++;
      }
    }
  }

  void handlePlus()
  {
    if (this->state_type == Insert)
    {
      this->handleDigit('+');
    }
  }

  void handleMinus()
  {
    if (this->state_type == Insert)
    {
      this->handleDigit('-');
    }
  }

  void handleLeft()
  {
    if (this->state_type == Insert)
    {
      if (this->state.on_insert.input_cursor == 0)
      {
        return;
      }
      this->state.on_insert.input_cursor--;

      if (this->state.on_insert.input_cursor < this->state.on_insert.print_offset)
      {
        // move a screen to the left, just like the TI-84
        this->state.on_insert.print_offset = (u8)max((int)this->state.on_insert.print_offset - SCREEN_WIDTH, 0);
      }
    }
  }

  void handleRight()
  {
    if (this->state_type == Insert)
    {
      if (this->state.on_insert.input_cursor >= this->line_size)
      {
        return;
      }
      this->state.on_insert.input_cursor++;

      if (this->state.on_insert.input_cursor >= this->state.on_insert.print_offset + SCREEN_WIDTH)
      {
        // move a unit right
        this->state.on_insert.print_offset++;
      }
    }
  }

  void handleEnter()
  {
    if (this->state_type == Insert)
    {
      if (this->line_size == 0) return;

      long res = 0;
      this->calculate(&res);

      this->history.addRecord(this->line_buffer, this->line_size, res);

      this->state_type = Select;
      this->state.on_select = {0, false};
      for (int i = 0; i < this->line_size; i++)
      {
        this->line_buffer[i] = 0;
      }
      this->line_size = 0;
    }
    else
    {
      // append selection from history to the line buffer
      bool on_question = this->state.on_select.selecting_question; 
      History::record r = this->history.getRecord(this->state.on_select.history_cursor);

      this->state_type = Insert;
      this->state.on_insert = {this->line_size, (u8)max((int)this->line_size - SCREEN_WIDTH + 1, 0)};

      u16 cursor = r.start;
      if (on_question)
      {
        for (int i = 0; i < r.prompt_len; i++)
        {
          this->handleDigit(this->history.buffer[cursor]);
          cursor = (cursor + 1) % History::BUFFER_LENGTH;
        }
      }
      else
      {
        cursor = (cursor + r.prompt_len) % History::BUFFER_LENGTH;
        for (int i = 0; i < r.answer_len; i++)
        {
          this->handleDigit(this->history.buffer[cursor]);
          cursor = (cursor + 1) % History::BUFFER_LENGTH;
        }
      }
    }
  }

  void handleUp()
  {
    if (this->state_type == Insert)
    {
      if (!this->history.isRecord(0))
        return;
      this->state_type = Select;
      this->state.on_select = {0, false};
    }
    else
    {
      if (this->state.on_select.selecting_question)
      {
        u8 next = this->state.on_select.history_cursor + 1;
        if (!this->history.isRecord(next))
          return;
        this->state.on_select.history_cursor = next;
        this->state.on_select.selecting_question = false;
      }
      else
      { // answer to question
        this->state.on_select.selecting_question = true;
      }
    }
  }

  void handleDown()
  {
    if (this->state_type == Select)
    {
      if (this->state.on_select.selecting_question)
      { // question to answer
        this->state.on_select.selecting_question = false;
      }
      else
      { // answer to question
        if (this->state.on_select.history_cursor == 0)
        {
          this->state_type = Insert;
          this->state.on_insert = {this->line_size, (u8)max((int)this->line_size - SCREEN_WIDTH + 1, 0)};
        }
        else
        {
          this->state.on_select.history_cursor = this->state.on_select.history_cursor - 1;
          this->state.on_select.selecting_question = true;
        }
      }
    }
  }

  void print(LiquidCrystal &lcd)
  {
    lcd.noBlink();
    lcd.clear();
    lcd.home();

    if (this->state_type == Insert)
    {
      // Serial.println(String("Buffer:") + this->line_buffer + ", Cursor_pos: " + this->state.on_insert.input_cursor + ", Size: " + this->line_size);

      for (int i = this->state.on_insert.print_offset; i < min(this->state.on_insert.print_offset + SCREEN_WIDTH, this->line_size); i++)
      {
        lcd.print(this->line_buffer[i]);
      }

      if (this->state.on_insert.print_offset > 0)
      {
        lcd.setCursor(0, 0);
        lcd.print('<');
      }

      if (this->state.on_insert.print_offset + SCREEN_WIDTH < this->line_size)
      {
        lcd.setCursor(SCREEN_WIDTH - 1, 0);
        lcd.print('>');
      }

      lcd.setCursor(this->state.on_insert.input_cursor - this->state.on_insert.print_offset, 0);
      lcd.blink();
    }
    else
    {
      History::record r = this->history.getRecord(this->state.on_select.history_cursor);

      // Serial.print(String("Record at ") + this->state.on_select.history_cursor + " => Prompt: ");

      u16 cursor = r.start;
      for (int i = 0; i < r.prompt_len; i++)
      {
        // Serial.print(this->history.buffer[cursor]);
        lcd.print(this->history.buffer[cursor]);
        cursor = (cursor + 1) % History::BUFFER_LENGTH;
      }

      lcd.setCursor(0, 1);
      Serial.print(", Answer: ");
      for (int i = 0; i < r.answer_len; i++)
      {
        // Serial.print(this->history.buffer[cursor]);
        lcd.print(this->history.buffer[cursor]);
        cursor = (cursor + 1) % History::BUFFER_LENGTH;
      }
      // Serial.println();

      if (this->state.on_select.selecting_question)
      {
        lcd.home();
        lcd.blink();
      }
      else
      {
        lcd.setCursor(0, 1);
        lcd.blink();
      }
    }
  }

private:
  bool calculate(long *out)
  {
    Token token;
    long res;
    long next_num;

    char *cursor = this->line_buffer;

    if (this->nextNumber(&next_num, &cursor))
    {
      res = next_num;
    }

    while ((token = this->next_token(cursor, &cursor)) != EOI)
    {
      if (token == Plus)
      {
        if (this->nextNumber(&next_num, &cursor))
        {
          res += next_num;
        }
        else
        {
          return false;
        }
      }
      else if (token == Minus)
      {
        if (this->nextNumber(&next_num, &cursor))
        {
          res -= next_num;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }

    *out = res;
    return true;
  }

  bool nextNumber(long *out, char **cursor)
  {
    Token token = this->next_token(*cursor, cursor);
    if (token == Num)
    {
      long res = strtol(*cursor, cursor, 10);
      *out = res;
      return true;
    }
    else if (token == Ans)
    {
      *out = 0; // TODO
      return true;
    }
    else if (token == Minus)
    {
      token = this->next_token(*cursor, cursor);
      if (token == Num)
      {
        *out = -strtol(*cursor, cursor, 10);
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }

  enum Token
  {
    EOI,
    Plus,
    Minus,
    Ans, // TODO
    Err, // TODO
    Num  // manually call strtoall
  };

  Token next_token(const char *str, char **endptr)
  {
    switch (*str)
    {
    case '\0':
      return EOI;
    case 'A':
      *endptr = str + 3;
      return Ans;
    case 'E':
      *endptr = str + 3;
      return Err;
    case '+':
      *endptr = str + 1;
      return Plus;
    case '-':
      *endptr = str + 1;
      return Minus;
    default:
      return Num;
    }
  }
};