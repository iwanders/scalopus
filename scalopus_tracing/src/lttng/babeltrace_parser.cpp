/*
  Copyright (c) 2018-2019, Ivor Wanders
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the name of the author nor the names of contributors may be used to
    endorse or promote products derived from this software without specific
    prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "scalopus_tracing/babeltrace_parser.h"
#include <iostream>
#include <sstream>

namespace scalopus
{
BabeltraceParser::BabeltraceParser()
{
  setLogger([](const std::string&) {});
}

CTFEvent BabeltraceParser::parse(std::string line)
{
  // Conversion is at the moment only offloaded to the CTFEvent class, but we may need some conversion here if we
  // are to deal with multiple outputs from babeltrace.
  return CTFEvent{ line };
}

void BabeltraceParser::setLogger(LoggingFunction logger)
{
  logger_ = logger;
}

void BabeltraceParser::process(FILE* stdout)
{
  if (stdout == nullptr)
  {
    logger_("[BabeltraceParser] Incorrect filepointer for stdout, quitting parser.");
    return;
  }

  // start the processing.
  processing_.store(true);
  std::array<char, 1024> tmp;
  int fd = fileno(stdout);
  fd_set read_fds;
  fd_set write_fds;
  fd_set except_fds;
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&except_fds);
  struct timeval tv;
  while (processing_.load())  // while processing.
  {
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    FD_SET(fd, &read_fds);
    FD_SET(fd, &except_fds);

    const int nfds = fd + 1;
    int select_result = select(nfds, &read_fds, &write_fds, &except_fds, &tv);
    if (select_result == -1)
    {
      continue;
    }
    if (FD_ISSET(fd, &read_fds))
    {
      // read line
      std::string line;
      if (std::fgets(tmp.data(), tmp.size(), stdout))
      {
        line = std::string(tmp.data());
      }
      if (!line.empty())
      {
        // lock, if recording sessions, parse and pass to all recording sessions.
        std::lock_guard<std::mutex> lock(mutex_);
        if (!sessions_recording_.empty())
        {
          auto event = parse(line);
          for (auto it = sessions_recording_.begin(); it != sessions_recording_.end();)
          {
            if ((*it)->active == false)
            {
              // session went inactive, remove it.
              std::stringstream ss;
              ss << "[BabeltraceParser] callback inactive: " << it->get();
              logger_(ss.str());
              it = sessions_recording_.erase(it);
              continue;
            }
            if ((*it)->callback)
            {
              (*it)->callback(event);
            }
            it++;
          }
        }
      }
      if (std::feof(stdout))
      {
        logger_("[BabeltraceParser] Reached end of file, quiting parser function.");
        processing_.store(false);
      }
    }
    if (FD_ISSET(fd, &except_fds))
    {
      logger_("[BabeltraceParser] Exception occured while reading from babeltrace stdout.");
      processing_.store(false);
    }
  };
}

bool BabeltraceParser::isProcessing()
{
  return processing_.load();
}

void BabeltraceParser::halt()
{
  processing_.store(false);
  std::lock_guard<std::mutex> lock(mutex_);
  sessions_recording_.clear();
}

void BabeltraceParser::setCallback(const std::shared_ptr<EventCallback>& callback)
{
  std::lock_guard<std::mutex> lock(mutex_);
  std::stringstream ss;
  ss << "[BabeltraceParser] Adding callback: " << callback.get();
  logger_(ss.str());
  sessions_recording_.insert(callback);
}

}  // namespace scalopus
