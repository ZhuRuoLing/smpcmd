//
// Created by jkl-9 on 2022/11/29.
//

#ifndef SMPCMD_SMPCMD_H
#define SMPCMD_SMPCMD_H

#pragma once

#include <functional>
#include <utility>
#include "iostream"
#include "tuple"
#include "vector"
#include "map"
#include "any"


enum class ArgType {
    IntegerArg = 114514,
    WordArg = 1919810,
    GreedyStringArg = 233,
    Undefined = 0
};

template<typename S>
class CommandContext {
private:
    S source;
    std::map<std::string, std::any> arguments;
public:

    explicit CommandContext(S source) : source(source) {}

    template<typename T>
    void addArgument(const std::string &name, T t) {
        arguments[name] = t;
    }

    [[nodiscard]] S &getSource() {
        return source;
    }

    void setSource(S src) {
        CommandContext::source = std::move(src);
    }

    [[nodiscard]] const std::map<std::string, std::any> &getArguments(){
        return arguments;
    }

    template<typename T>
    [[nodiscard]] T getArgument(const std::string& argumentName){
        return any_cast<T>(arguments[argumentName]);
    }

};

template<typename S>
class Node {
private:
    std::map<std::string, Node<S>> children;
    std::string literal;
    ArgType argType = ArgType::Undefined;
    std::function<uint32_t(CommandContext<S>)> function;
    bool hasChild = false;
public:
    Node() {
        hasChild = false;
    }

    Node<S> &then(Node<S> child) {
        children[child.literal] = child;
        hasChild = true;
        return *this;
    };

    Node<S> &asLiteral(const std::string& string){
        this->literal = string;
        return *this;
    }

    Node<S> &executes(std::function<uint32_t(CommandContext<S>)> func) {
        this->function = func;
        hasChild = false;
        return *this;
    }

    [[nodiscard]] bool isHasChild() {
        return hasChild;
    }

    Node<S> &asArgument(ArgType argT, const std::string &argName) {
        this->literal = "<" + argName + ">";
        this->argType = argT;
        return *this;
    }

    std::map<std::string, Node<S>> &getChildren() {
        return children;
    }

    [[nodiscard]] const std::string &getLiteral() {
        return literal;
    }

    [[nodiscard]] ArgType getArgType() {
        return argType;
    }

    [[nodiscard]] const auto &getFunction() {
        return function;
    }
};

constexpr int commandError = -114514;

template<typename S>
class CommandDispatcher {
private:
    std::map<std::string, Node<S>> roots;
public:
    std::vector<std::string> split(const std::string &s, const std::string &seperator) {
        std::vector<std::string> result;
        typedef std::string::size_type string_size;
        string_size i = 0;
        while (i != s.size()) {
            int flag = 0;
            while (i != s.size() && flag == 0) {
                flag = 1;
                for (char x: seperator)
                    if (s[i] == x) {
                        ++i;
                        flag = 0;
                        break;
                    }
            }
            flag = 0;
            string_size j = i;
            while (j != s.size() and flag == 0) {
                for (char x: seperator)
                    if (s[j] == x) {
                        flag = 1;
                        break;
                    }
                if (flag == 0)
                    ++j;
            }
            if (i != j) {
                result.push_back(s.substr(i, j - i));
                i = j;
            }
        }
        return result;
    }

    int dispatch(const std::string &command, S source) {
        std::vector<std::string> vector = split(command, " ");
        if (roots.count(vector.front()) > 0) {
            auto node = roots[vector.front()];
            auto commandContext = CommandContext(source);
            return execute(node, vector, commandContext);
        } else {
            return commandError;
        }
    }
    void addChild(Node<S> &nod) {
        roots[nod.getLiteral()] = nod;
    }
private:
    int execute(Node<S> node, const std::vector<std::string> &command, CommandContext<S> commandContext) {
        auto argType = node.getArgType();
        if (argType == ArgType::Undefined) {//a literal node
            auto children = node.getChildren();
            if (children.size() > 0) {
                std::vector<std::string> cp(command);
                //std::copy(command.begin(), command.end(), cp.begin());
                std::string first = *cp.begin();
                if (node.getChildren().count(first) > 0) {
                    auto n = node.getChildren()[first];
                    cp.erase(cp.cbegin());
                    return execute(n, cp, commandContext);
                } else {
                    return commandError;
                }
            } else {
                auto func = node.getFunction();
                if (func) {
                    return func(commandContext);
                } else {
                    return commandError;
                }
            }
        } else {//a argument
            if (node.getChildren().size() > 0) {
                auto arg = command.begin().operator*();
                auto children = node.getChildren();
                if (children.count(arg) > 0) {
                    auto next = children[arg];
                    if (argType == ArgType::IntegerArg) {
                        int val = std::stoi(arg);
                        commandContext.addArgument(node.getLiteral(), val);
                    } else {
                        if (argType == ArgType::WordArg) {
                            commandContext.addArgument(node.getLiteral(), arg);
                        } else {
                            std::string str;
                            for (const auto &item: command){
                                str.append(item);
                            }
                            commandContext.addArgument(node.getLiteral(), arg);
                        }
                    }
                    std::vector<std::string> cp;
                    std::copy(command.begin(), command.end(), cp.begin());
                    cp.erase(cp.cbegin());
                    return execute(next, cp, commandContext);
                } else {
                    return commandError;
                }
            } else {
                auto arg = command.cbegin().operator*();
                if (argType == ArgType::IntegerArg) {
                    int val = std::stoi(arg);
                    commandContext.addArgument(node.getLiteral(), val);
                } else {
                    if (argType == ArgType::WordArg) {
                        commandContext.addArgument(node.getLiteral(), arg);
                    } else {
                        std::string str;
                        for (const auto &item: command){
                            str.append(item);
                        }
                        commandContext.template addArgument(node.getLiteral(), arg);
                    }
                }
                auto func = node.getFunction();
                if (func) {
                    return func(commandContext);
                } else {
                    return commandError;
                }
            }
        }
    }
};

template<typename S, typename T>
T getArgument(CommandContext<S> &context, const std::string &argName) {
    std::any any = context.getArguments()[argName];
    return static_cast<T>(any);
}

template<typename S>
uint32_t getIntegerArgument(CommandContext<S> &context, const std::string &argName) {
    return getArgument<S, uint32_t>(context, "<" + argName + ">");
}

template<typename S>
std::string getStringArgument(CommandContext<S> &context, const std::string& argName) {
    return getArgument<S, std::string>(context, "<" + argName + ">");
}

#endif //SMPCMD_SMPCMD_H
