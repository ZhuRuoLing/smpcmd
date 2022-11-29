
#include <iostream>
#include "smpcmd.h"

class CommandSource {
    int source;
public:
    explicit CommandSource(int source) : source(source) {}

    int getSource() const {
        return source;
    }

    void setSource(int source) {
        CommandSource::source = source;
    }
};


int main() {
    Node<CommandSource> node = Node<CommandSource>().asLiteral("wdnmd")
            .then(
                    Node<CommandSource>().asArgument(ArgType::WordArg, "some_arg").executes(
                            [](CommandContext<CommandSource> context) -> int {
                                std::cout << "called wdnmd with arg" << context.getArgument<std::string>("some_arg") << " and command source is" << context.getSource().getSource() << std::endl;
                                return 0;
                            })
            ).executes([](CommandContext<CommandSource> context){
                std::cout << "called wdnmd with no args"<< " and command source is" << context.getSource().getSource() << std::endl;
                return 1;
            });

    CommandDispatcher<CommandSource> dispatcher;
    dispatcher.addChild(node);
    int i = dispatcher.dispatch("wdnmd", CommandSource(114514));
    int r = dispatcher.dispatch("wdnmd wcnmb", CommandSource(1919810));
    std::cout << i << " " << r << std::endl;
 }
