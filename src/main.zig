const std = @import("std");

pub fn main() !void {
    const stdout = std.io.getStdOut().writer();
    const stdin = std.io.getStdIn().reader();

    while (true) {
        try stdout.writeAll("$ ");

        var bufferLine: [100]u8 = undefined;
        if (try stdin.readUntilDelimiterOrEof(&bufferLine, '\n')) |userInput| {
            const trimmed = std.mem.trim(u8, userInput, &std.ascii.whitespace);

            if (std.mem.eql(u8, trimmed, "exit")) {
                break;
            }
        }
    }
}
