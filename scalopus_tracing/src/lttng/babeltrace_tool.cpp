/*
  Copyright (c) 2018, Ivor Wanders
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

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

#include "scalopus_tracing/babeltrace_tool.h"

namespace scalopus
{
BabeltraceTool::BabeltraceTool()
{
  parser_ = std::make_shared<BabeltraceParser>();
}

void BabeltraceTool::init(std::string path)
{
  if (process_ != nullptr)
  {
    return;  // need to halt first, just call it a day, don't want to get dangling workers / threads / viewers.
  }

  if (path.empty())
  {
    // lttng view call, eliminates need for hostname:$ lttng view test_session -e "babeltrace --input-format=lttng-live"
    process_ =
        std::make_shared<subprocess::popen>("lttng", std::vector<std::string>{ "view", "scalopus_target_session", "-e",
                                                                               "babeltrace --clock-seconds --clock-gmt "
                                                                               "--no-delta "
                                                                               "--input-format=lttng-live" });
  }
  else
  {
    // Direct babeltrace call:$ babeltrace -v --input-format=lttng-live
    // net://localhost/host/eagle/scalopus_target_session
    process_ = std::make_shared<subprocess::popen>(
        "babeltrace",
        std::vector<std::string>{ "--clock-seconds", "--clock-gmt", "--no-delta", "--input-format=lttng-live", path });
  }

  // Start processing the output of stdout in the parser process function, do this in a separate thread.
  parser_worker_ = std::thread([&]() { parser_->process(process_->stdoutFile()); });
}

BabeltraceTool::~BabeltraceTool()
{
  halt();
}

void BabeltraceTool::halt()
{
  // first stop the parser
  parser_->halt();
  if (parser_worker_.joinable())
  {
    parser_worker_.join();
  }

  // then halt the babeltrace process.
  if (process_)
  {
    process_->close();

    // Nuke the process, just to be sure we clean up correctly.
    ::kill(process_->getPid(), SIGINT);

    process_ = nullptr;
  }
}

std::shared_ptr<BabeltraceParser::EventCallback>
BabeltraceTool::addCallback(std::function<void(const CTFEvent& event)> fun)
{
  if (!parser_->isProcessing())
  {
    std::cout << "Making a session on a parser that's not processing." << std::endl;
  }
  // make a new session
  auto session = std::make_shared<BabeltraceParser::EventCallback>();
  session->callback = fun;

  parser_->setCallback(session);
  return session;
}

}  // namespace scalopus
