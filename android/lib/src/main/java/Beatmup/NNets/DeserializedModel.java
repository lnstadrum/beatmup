/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package Beatmup.NNets;

import Beatmup.Context;
import Beatmup.Exceptions.CoreException;

/**
 * Model reconstructed from a serialized representation.
 * The representation format is the one rendered with Model::serialize(): a YAML-like listing containing "ops" and "connections" sections
 * describing the model operations in execution order and connections between them respectively (see \ref NNetsModelSerialization).
 */
public class DeserializedModel extends Model {
    private static native long newDeserializedModel(Context context, String str) throws CoreException;

    /**
     * Constructs a model from its serialized representation.
     * The expected representation format is the one rendered with {@link Model#serializeToString}.
     * @param context       A context instance the model resources are bound to
     * @param str           A string containing the model representation
     */
    public DeserializedModel(Context context, String str) throws  CoreException {
        super(newDeserializedModel(context, str));
    }
}
