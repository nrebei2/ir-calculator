class History
{
public:
  static const u16 BUFFER_LENGTH = 900;
  static const u16 HISTORY_LENGTH = 30;
  // segment of buffer, not necessarily contiguous
  struct record
  {
    long answer;   // cached
    u16 start;     // offset from buffer
    u8 prompt_len;
    u8 answer_len;
  };
  char buffer[History::BUFFER_LENGTH]; // circular buffer
private:
  record history[History::HISTORY_LENGTH]; // circular queue
  u8 start_idx = 0;
  u8 len = 0;

public:
  void addRecord(char *prompt, u8 prompt_len, long answer)
  {
    u16 place_at;
    if (this->len == 0)
    {
      place_at = 0;
    }
    else
    {
      record latest_record = this->history[(this->start_idx + len - 1) % History::HISTORY_LENGTH];
      place_at = (latest_record.start + latest_record.prompt_len + latest_record.answer_len) % History::BUFFER_LENGTH;
    }

    // snprintf appends \0 so put into temp buffer first
    char answer_buffer[12];
    u8 answer_len = snprintf(answer_buffer, 12, "%ld", answer);

    u16 place_till = (place_at + prompt_len + answer_len) % History::BUFFER_LENGTH;
    bool wraps = place_till < place_at;

    // clear overlapping early records
    while (this->len != 0)
    {
      u16 early_start = this->getEarliestRecord().start;
      if (wraps)
      {
        if (early_start >= place_at || early_start < place_till)
        {
          this->popEarliestRecord();
          continue;
        }
      }
      else
      {
        if (early_start >= place_at && early_start < place_till)
        {
          this->popEarliestRecord();
          continue;
        }
      }
      break;
    }

    if (this->len == History::HISTORY_LENGTH)
    { // still full
      popEarliestRecord();
    }

    // copy to buffer
    if (wraps)
    {
      bool placing_prompt = true;
      u16 cursor = place_at;
      u16 cur_offset = 0;

      while (true)
      {
        if (placing_prompt)
        {
          if (cur_offset == prompt_len)
          {
            placing_prompt = false; // done
            cur_offset = 0;
            continue;
          }
          else
          {
            this->buffer[cursor] = prompt[cur_offset];
          }
        }
        else
        {
          // placing answer
          if (cur_offset == answer_len)
          {
            break;
          }
          else
          {
            this->buffer[cursor] = answer_buffer[cur_offset];
          }
        }
        cur_offset++;
        cursor = (cursor + 1) % History::BUFFER_LENGTH;
      }
    }
    else
    {
      u16 cursor = place_at;
      memcpy(this->buffer + cursor, prompt, prompt_len);
      cursor += prompt_len;
      memcpy(this->buffer + cursor, answer_buffer, answer_len);
    }

    // add record to history
    this->history[(this->start_idx + len) % History::HISTORY_LENGTH] = {answer, place_at, prompt_len, answer_len};
    this->len++;
  }

  bool isRecord(u8 at)
  {
    return at < this->len;
  }

  // at = 0 => latest record => index = this->start_idx + len - 1
  record getRecord(u8 at)
  {
    int idx = (((int)this->start_idx + len - 1 - at) % History::HISTORY_LENGTH + History::HISTORY_LENGTH) % History::HISTORY_LENGTH;
    return this->history[idx];
  }

private:
  // precondition: not empty
  void popEarliestRecord()
  {
    this->start_idx++;
    this->len--;
  }

  // precondition: not empty
  record getEarliestRecord()
  {
    return this->history[this->start_idx];
  }
};